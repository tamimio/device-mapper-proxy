Device mapper proxy to monitor the statistic of performed on the device operations.

# Usage
To build and test module run `make test`.

Building and installation of the module can also be completed step-by-step:
1. To compile the module run `make`;
2. To install the module run `sudo insmod dmp.ko` or `make install`;
3. To create devices run `make setup`;
4. Then you can use `dd`to test module.
   Additionally, you can test the module using FIO utility (via block.fio jobfile) by running `make fio`.

To clean the directory and to unload module run `make unload` and `make clean`.
