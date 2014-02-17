pointsinboxes
=============

Problem Statement
=================

Input

You are given a text file. The file consists of two types of lines:

* Lines starting with the keyword _point_: The syntax is point, whitespace, double, whitespace, double, whitespace, string.
* Line starting with the keyword _box_: The syntax is box, whitespace, double, whitespace, double, whitespace, double, whitespace double.

For example:

```
point 7.08 0.64 358b7990-387c-43a5-92d8-b454a727bf63
point -8.87 -1.00 172e5c20-041a-441b-a036-59fa08c48525
point 7.23 1.92 ea1dd912-b4a2-490a-8969-65e45d25e7cf
point 7.24 2.25 4be97075-2612-4e35-8598-945a37966eeb
box 7 0 8 2
point 7.02 -9.46 16e2f9b8-298c-40cf-aeae-219e95cd5caa
point 8.73 10.37 58ef0ff7-6478-4891-90ae-3ab442a118cf
point 0.14 6.51 fd5d0ff6-e7ad-4992-b237-56f3b4da75e8
point 7.73 3.76 0e0c7ce1-9045-4c6d-879a-1ed0bcbdf03c
box 7.2 2.0 7.3 0.5
point -1.98 6.74 be63d02c-5592-4786-be92-45278eaf1329
point -1.50 -8.82 da7790e3-4904-44a5-a02f-0384fcaef902
point 5.00 8.18 4fa09431-9fe9-4fd8-a3b7-8c2a0d0f7723
point 0.78 5.16 b8537460-7d63-4ff1-b3a4-f5abbbbc8a16
box 10 10 11 11
point 0.32 -9.42 605714b1-b537-49cb-8e0b-4ce6c1b8e208
point 3.60 -5.93 948b23d9-47aa-4121-b7fb-3f23e7dff259
box -3.1 -8.9 3.5 6.6
point -2.43 1.36 b09dbbc7-4ddb-4cb4-9b06-6fe24a3ee1e2
```

A point line should be interpreted as a point in a plane where the
first number represents the point's x coordinate, the second number
its y coordinate and the final string its ID. A box line should be interpreted
as axis-parallel bounding box with first and third number being
its x boundaries and second and fourth number its y boundaries.

Problem:

Write a program that parses a file as described above and for
each box line prints out this line followed by the IDs of points inside the box
that appeared before the line.

For example for above file the program should print:

```
box 7 0 8 2
358b7990-387c-43a5-92d8-b454a727bf63
ea1dd912-b4a2-490a-8969-65e45d25e7cf
box 7.2 2.0 7.3 0.5
ea1dd912-b4a2-490a-8969-65e45d25e7cf
box 10 10 11 11
box -3.1 -8.9 3.5 6.6
fd5d0ff6-e7ad-4992-b237-56f3b4da75e8
da7790e3-4904-44a5-a02f-0384fcaef902
b8537460-7d63-4ff1-b3a4-f5abbbbc8a16
```

Your implementation should assume that the number of points is much
higher than the number of boxes. You should aim for a time-efficient
solution.

Constraints

* Language: Any of the following: C++, Java, Ruby, Python
* Platform: OS X or Linux
* Document your code
* Provide a self-contained folder:
    * Provide an INSTALL file that explains how to build and run your program
    * There should be no additional requirements on the system.

Intructions
===========

This implementation builds and runs on Ubuntu Linux. It has not been tested on anything else.

To build, run:

    make

To generate input file run the following command:

    ./generate-input-file min_x min_y maxs_ x maxs_y number_of_points number_of_boxes

example to generate 10 points and 5 boxes inside (-3, -3) and (6, 6) coordinates:

    ./generate-input-file -3 -3 6 6 10 5

This will generate output file in data.out file. To run the program with this data file do the following:

    ./points-in-boxes data.out > outputfile

Input File Format
=================

An input file can be generated as described above. If you want to run it with your own file,
keep in mind that an identifier of a point is assumed to be a UUID represented as a string of 36 characters.

Performance
===========

In the attempt to solve this problem I tried out several ideas before I settled down on the present version.
Initially I thought that finding the points in the boxes is the going to be the most time consuming operation.
So, I focused my attention on finding the best performing algorithm. So, when I finally settled down on the one
that performs the best of all, it turned out that the algorithm to find points in the boxes does not take much time
compared to the time it takes to parse the input file. It was especially apparent when I generated the data set of
100000 points and 100 boxes. It takes close to 30 seconds to parse this file when the file is fully memory mapped.
The time it takes to do everything else is negligible. Here is an example of the my diagnostic output for this big
data set:

```
Mapping file to memory took: 0.004286562 seconds
Parsing file from memory took: 30.833523657 seconds
Processing data took: 0.222577624 seconds
Printing results took: 0.803702772 seconds
```

Additional Information
======================

As it turns out the example result in the problem statement is not entirely correct. The last box is missing point id:
'<b09dbbc7-4ddb-4cb4-9b06-6fe24a3ee1e2>' as it should be also inside this box.
