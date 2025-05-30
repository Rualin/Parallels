
// nvtxRangePushA("init");
    // double corners[] = {10.0, 20.0, 30.0, 20.0};
    // memset(A, 0, n * n * sizeof(double));
    // A[0] = corners[0];
    // A[n - 1] = corners[1];
    // A[n * n - 1] = corners[2];
    // A[n * (n - 1)] = corners[3];

    // double number_of_steps = n - 1;
    // double top_side_step = (double)abs(corners[0] - corners[1]) / number_of_steps;
    // double right_side_step = (double)abs(corners[1] - corners[2]) / number_of_steps;
    // double bottom_side_step = (double)abs(corners[2] - corners[3]) / number_of_steps;
    // double left_side_step = (double)abs(corners[3] - corners[0]) / number_of_steps;

    // double top_side_min = std::min(corners[0], corners[1]);
    // double right_side_min = std::min(corners[1], corners[2]);
    // double bottom_side_min = std::min(corners[2], corners[3]);
    // double left_side_min = std::min(corners[3], corners[0]);

    // for (int i = 1; i < n - 1; i++) {
    //     A[i] = top_side_min + i * top_side_step;
    //     A[n * i] = left_side_min + i * left_side_step;
    //     A[(n - 1) + n * i] = right_side_min + i * right_side_step;
    //     A[n * (n - 1) + i] = bottom_side_min + i * bottom_side_step;
    // }
    // std::memcpy(Anew, A, n * n * sizeof(double));
    // nvtxRangePop();



        // if (!graph_created) {
        //     create_graph(stream, graph, instance, d_A, d_Anew);
        //     // nvtxRangePushA("createGraph");
        //     // // начало захвата операций на потоке stream
        //     // cudaErr = cudaStreamBeginCapture(stream, cudaStreamCaptureModeGlobal);
        //     // if (cudaErr != cudaSuccess) f_exception(cudaStreamBeginCapture_err);
        //     // for (int i = 0; i < 100; i++)
        //     //     calc_mean<<<grid, block, 0, stream>>>(d_A, d_Anew, n, (i % 2 == 1));
        //     // // завершение захвата операций
        //     // cudaErr = cudaStreamEndCapture(stream, &graph);
        //     // if (cudaErr != cudaSuccess) f_exception(cudaStreamEndCapture_err);
        //     // nvtxRangePop();
        //     // // создаем исполняемый граф
        //     // cudaErr = cudaGraphInstantiate(&instance, graph, NULL, NULL, 0);
        //     // if (cudaErr != cudaSuccess) f_exception(cudaGraphInstantiate_err);
        //     graph_created = true;
        // }


