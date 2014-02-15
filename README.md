pointsinboxes
=============

Intructions
===========

To build, run:

make

To generate input file run the following command:

./generate-input-file <min x> <min y> <max x> <max y> <number of points> <number of boxes>

example to generate 10 points and 5 boxes inside (-3, -3) and (6, 6) coordinates:

./generate-input-file -3 -3 6 6 10 5

This will generate output file in data.out file. To run the program with this data file do the following:

./points-in-boxes data.out

