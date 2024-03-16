#!/bin/bash
for i in {1..5}
do
    echo "Run $i"

    # execute the code
    ./test1

    # check if the execution was successful and the output files are not empty
    while [ $? -ne 0 ] || [ ! -s out-tas.txt ] || [ ! -s out-cas.txt ] || [ ! -s out-bounded.txt ]
    do
        echo "Execution failed or output files are empty, retrying..."
        ./test1
    done    

    # get the last two lines from each file
    last_two_lines_tas=$(tail -n 2 out-tas.txt)
    last_two_lines_cas=$(tail -n 2 out-cas.txt)
    last_two_lines_bounded=$(tail -n 2 out-bounded.txt)

    # extract the overall average and maximum time from the last two lines
    avg_time_tas=$(echo "$last_two_lines_tas" | grep -oP 'Overall Average time to enter CS for all threads is \K[0-9.]+')
    max_time_tas=$(echo "$last_two_lines_tas" | grep -oP 'Overall Maximum time to enter CS for all threads is \K[0-9.]+')

    avg_time_cas=$(echo "$last_two_lines_cas" | grep -oP 'Overall Average time to enter CS for all threads is \K[0-9.]+')
    max_time_cas=$(echo "$last_two_lines_cas" | grep -oP 'Overall Maximum time to enter CS for all threads is \K[0-9.]+')

    avg_time_bounded=$(echo "$last_two_lines_bounded" | grep -oP 'Overall Average time to enter CS for all threads is \K[0-9.]+')
    max_time_bounded=$(echo "$last_two_lines_bounded" | grep -oP 'Overall Maximum time to enter CS for all threads is \K[0-9.]+')

    # append these values to the csv file
    echo "$avg_time_tas,$max_time_tas,$avg_time_cas,$max_time_cas,$avg_time_bounded,$max_time_bounded" >> output.csv
done