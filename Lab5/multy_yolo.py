import multiprocessing
import queue
import argparse
from dataclasses import dataclass, field
from typing import Any
import time
import timeit
import os

import ultralytics
import cv2


# print(multiprocessing.get_start_method())
if multiprocessing.get_start_method() != "spawn":
    multiprocessing.set_start_method("spawn")

OUTPUT_SHAPE = (640, 480)
NUM_YOLO_PROCESSES = 8

@dataclass(order=True)
class PrioritizedItem:
    priority: int
    item: Any = field(compare=False)


class Reader:
    def __init__(self, video_path):
        self.cap = cv2.VideoCapture(video_path)
        if not(self.cap.isOpened()):
            print("Video couldn't open!!!")
            self.cap.release()
            exit(1)
    
    def read(self):
        return self.cap.read()
    
    def __del__(self):
        self.cap.release()


class Writer:
    def __init__(self, output_name):
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        self.writer = cv2.VideoWriter(output_name + ".mp4", fourcc, 60, OUTPUT_SHAPE)
        if not(self.writer.isOpened()):
            print("Video writer couldn't open!!!")
            self.writer.release()
            exit(1)
    
    def write(self, frame):
        self.writer.write(frame)
    
    def __del__(self):
        self.writer.release()


class YoloProcess(multiprocessing.Process):
    def __init__(self, event, read_q, res_q, **process_args):
        multiprocessing.Process.__init__(self, **process_args)
        self.model = ultralytics.YOLO("yolov8s-pose.pt")
        self.event = event
        self.read_queue = read_q
        self.res_queue = res_q
        # self.read_queue = read_queue

    def run(self):
        print(f"Start of YOLO process {multiprocessing.current_process().name}", flush=True)
        while True:
            try:
                frame = self.read_queue.get(block=False)
                frame_num = frame[0]
                frame = frame[1]
                predicted = self.model.predict(frame, verbose=False)[0].plot()
                self.res_queue.put((frame_num, predicted), False)
            except queue.Empty:
                pass
            if self.event.is_set():
                    break
        print(f"End of YOLO process {multiprocessing.current_process().name}", flush=True)


# class WriteProcess(multiprocessing.Process):
#     def __init__(self, out_name, cond_var, **process_args):
#         multiprocessing.Process.__init__(self, **process_args)
#         # self.writer = Writer(out_name)
#         fourcc = cv2.VideoWriter_fourcc(*'mp4v')
#         self.writer = cv2.VideoWriter(output_name + ".mp4", fourcc, 30, OUTPUT_SHAPE)
#         if not(self.writer.isOpened()):
#             print("Video writer couldn't open!!!")
#             self.writer.release()
#             exit(1)
#         self.priquy = queue.PriorityQueue()
#         self.get_num = 0
#         self.cond_var = cond_var
    
#     def run(self):
#         print(f"Start of write process {multiprocessing.current_process().name}", flush=True)
#         global res_queue
#         while True:
#             try:
#                 frame = res_queue.get(False)
#                 frame_num = frame[0]
#                 frame = frame[1]
#                 self.priquy.put(PrioritizedItem(frame_num, frame), False)
#             except queue.Empty:
#                 pass
            
#             try:
#                 frame = self.priquy.get(False)
#                 frame_num = frame.priority
#                 # self.priquy.task_done()
#                 if frame_num != self.get_num:
#                     self.priquy.put(frame, False)
#                 else:
#                     self.writer.write(frame.item)
#                     self.get_num += 1
#             except queue.Empty:
#                 pass
            
#             with self.cond_var:
#                 if self.cond_var.wait(0.0001):
#                     break
#         print(f"End of write process {multiprocessing.current_process().name}", flush=True)
    
#     def __del__(self):
#         self.writer.release()


def multiproc_main(video_path, output_name):
    # cond_var = multiprocessing.Condition()
    event = multiprocessing.Event()
    cap = Reader(video_path)
    writer = Writer(output_name)
    put_num = 0
    get_num = 0
    priquy = queue.PriorityQueue()
    read_queue = multiprocessing.Queue()
    res_queue = multiprocessing.Queue()
    is_end = False
    processes = []
    for _ in range(NUM_YOLO_PROCESSES):
        proc = YoloProcess(event, read_queue, res_queue)
        processes.append(proc)
    # proc = WriteProcess(output_name, cond_var)
    # processes.append(proc)
    for proc in processes:
        proc.start()
    
    print("Before main while", flush=True)
    while True:
        success, frame = cap.read()
        if success:
            read_queue.put((put_num, frame), False)
            put_num += 1
        else:
            # print("End of reading", flush=True)
            is_end = True
            # print(put_num, flush=True)

        try:
            processed = res_queue.get(False)
            # print("Extracted from res_queue:", processed[0], flush=True)
            priquy.put(PrioritizedItem(processed[0], processed[1]), False)
        except queue.Empty:
            pass

        try:
            curr = priquy.get(False)
            # print("After priquy get", flush=True)
            prior = curr.priority
            if prior != get_num:
                priquy.put(curr, False)
            else:
                curr = curr.item
                writer.write(curr)
                get_num += 1
                cv2.imshow("Window", curr)
        except queue.Empty:
            pass
        
        if cv2.waitKey(1) == ord("q"):
            break
        if is_end and (put_num <= get_num):
            break
    print("After main while", flush=True)
    # with cond_var:
    #     cond_var.notify(NUM_YOLO_PROCESSES)
    event.set()

    for proc in processes:
        proc.join()
    
    cv2.destroyAllWindows()


def oneproc_main(video_path, output_name):
    cap = Reader(video_path)
    writer = Writer(output_name)
    model = ultralytics.YOLO("yolov8s-pose.pt")
    while True:
        success, frame = cap.read()
        if success:
            processed = model.predict(frame, verbose=False)[0].plot()
            writer.write(processed)
        else:
            break
    

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--video_path", "-vp", type=str, default="funny_out.mp4")
    parser.add_argument("--is_multiproc", "-is_m", type=bool, default=False)
    parser.add_argument("--output_name", "-on", type=str, default="out")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    video_path = args.video_path
    video_path = 0 if video_path == "0" else video_path
    is_multiproc = args.is_multiproc
    output_name = args.output_name
    if os.path.exists(output_name + ".mp4"):
        os.remove(output_name + ".mp4")

    start_time = time.time()
    start_timeit = timeit.default_timer()
    if is_multiproc:
        multiproc_main(video_path, output_name)
    else:
        oneproc_main(video_path, output_name)
    end_time = time.time()
    end_timeit = timeit.default_timer()
    dur_time = end_time - start_time
    dur_timeit = end_timeit - start_timeit
    print("Duration by time:", dur_time)
    print("Duration by timeit:", dur_timeit)
