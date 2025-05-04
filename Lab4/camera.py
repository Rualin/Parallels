import threading
import queue
import time
import logging
import argparse

import cv2
import numpy as np


MIN_SHAPE = 480

logger = logging.getLogger(__name__)
logger.setLevel(logging.WARNING)
handler = logging.FileHandler('logs\\log-file.txt')
formatter = logging.Formatter("%(name)s %(asctime)s %(levelname)s %(message)s")
handler.setFormatter(formatter)
logger.addHandler(handler)
logger_lock = threading.Lock()

stop_sensors = False

def custom_hook(args):
    with logger_lock:
        logger.error(f"{args.thread.name}:{args.exc_type}\n{args.exc_traceback}")

threading.excepthook = custom_hook

class ReadingFrameError(Exception):
    """cv2.VideoCapture.read() returned [False, ...]"""
    def __init__(self, *args):
        super().__init__(*args)


class Sensor:
    def get():
        raise NotImplementedError("Subclasses must implement method get")
    
class SensorX(Sensor):
    def __init__(self, delay):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data


class SensorCam(Sensor):
    def __init__(self, name, resolution: np.ndarray):
        self.cap = cv2.VideoCapture(name)
        if not self.cap.isOpened():
            raise ValueError('Cannot open camera')
        self.win_shape = resolution

    def get(self) -> cv2.typing.MatLike:
        # print("Start getting frame")
        success, frame = self.cap.read()
        if not success:
            self.cap.release()
            raise ReadingFrameError('Cannot read frame')
        # print("End getting frame")
        return cv2.resize(frame, self.win_shape, interpolation = cv2.INTER_AREA)
        # return frame
    
    def __del__(self):
        self.cap.release()


class SensorThread(threading.Thread):
    def __init__(self, type: str, *args, **thread_args):
        threading.Thread.__init__(self, **thread_args)
        if type == 'Cam':
            self.sensor = SensorCam(*args)
        elif type == 'X':
            self.sensor = SensorX(*args)
        else:
            raise ValueError("Wrong type of Sensor")
        self.q = queue.Queue()
        
    def run(self):
        global stop_sensors, logger, logger_lock
        while not(stop_sensors):
            # print("Start while")
            try:
                self.q.put(self.sensor.get())
                # print(self.q.qsize())
            except ReadingFrameError:
                with logger_lock:
                    logger.exception(f"{threading.current_thread().name}: SensorCam couldn't read next frame. Maybe connection was broken")
                break
            except AttributeError:
                print("Attribute error")
            # print("End while")
        # print("After while")
        while self.q.not_empty:
            self.q.get(False)
            self.q.task_done()
        self.q.join()

    def get(self):
        # print("Frame getted")
        return self.q.get(False)
    
    def task_done(self):
        self.q.task_done()

class WindowImage():
    def __init__(self, freq: int):
        if freq < 0:
            logger.warning(f"{threading.current_thread().name}: The output frequency must be > 0 but got {freq}. Now it is set to 1")
            freq = 1
        self.delay = 1.0 / freq
        self.winname = 'out'
        try:
            cv2.namedWindow(self.winname, cv2.WINDOW_AUTOSIZE)
        except (...):
            logger.exception(f"{threading.current_thread().name}: Couldn't create output window.")
            global stop_sensors
            stop_sensors = True

    def show(self, image):
        time.sleep(self.delay)
        try:
            cv2.imshow(self.winname, image)
        except (...):
            logger.exception(f"{threading.current_thread().name}: Couldn't show current frame.")
            global stop_sensors
            stop_sensors = True

    def __del__(self):
        cv2.destroyAllWindows()


def make_output_image(frame: cv2.typing.MatLike, sensor_values: np.ndarray):
    res = cv2.rectangle(frame, (0, 0), (170, 70), (255, 255, 255), -1)
    for i, val in enumerate(sensor_values):
        res = cv2.putText(res, f'SensorX {i}: {val}', (10, 20 * (i + 1)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1)
    return res


def parse_args():
    parser = argparse.ArgumentParser(description='Read frames with different frequency from camera by threads')
    parser.add_argument('--camera_name', "-cn", type=str,
                        help='System name of a camera or its number')
    parser.add_argument('--window_width', "-ww", type=int,
                        help="Width of the output window, must be >= 128",
                        default=128)
    parser.add_argument('--window_height', "-wh", type=int,
                        help="Height of the output window, must be >= 64",
                        default=64)
    parser.add_argument('--freq', type=int,
                        help='Output window frequency',
                        default=30)
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    camera_shape = np.array([args.window_width, args.window_height], dtype=int)
    if (camera_shape < MIN_SHAPE).any():
        with logger_lock:
            logger.warning(f'{threading.current_thread().name}: All sizes of output window must be >= {MIN_SHAPE}')
        camera_shape = np.array([MIN_SHAPE, MIN_SHAPE], dtype=int)
    camera_name = args.camera_name if args.camera_name is not None else 0
    try:
        sensor_cam = SensorThread('Cam', camera_name, camera_shape)
    except ValueError:
        logger.exception(f'{threading.current_thread().name}: ValueError')
        exit(1)

    sensor_threads = [
        SensorThread('X', 0.01),
        SensorThread('X', 0.1),
        SensorThread('X', 1)
    ]

    sensor_cam.start()
    for sensor_thread in sensor_threads:
        sensor_thread.start()
    
    window = WindowImage(args.freq)

    sensor_values = np.zeros((3), dtype=np.int32)
    frame = np.zeros((camera_shape[0], camera_shape[1]), dtype=np.uint8)

    while True:
        try:
            frame = sensor_cam.get()
            sensor_cam.task_done()
        except ValueError:
            break
        except queue.Empty:
            pass

        for i, sensor_thread in enumerate(sensor_threads):
            try:
                sensor_value = sensor_thread.get()
                sensor_thread.task_done()
                sensor_values.put([i], [sensor_value])
            except ValueError:
                break
            except queue.Empty:
                pass
        
        window.show(make_output_image(frame, sensor_values))
        if cv2.waitKey(1) == ord("q"):
            break
    
    stop_sensors = True

    for sensor_thread in sensor_threads:
        sensor_thread.join()
    
