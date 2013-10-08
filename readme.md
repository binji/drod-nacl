This is a NaCl port of DROD: Gunthro and the Epic Blunder.
See readme.DROD.txt for more info.

Setting up the Repo
-------------------

You'll probably need linux (or something linux-like).

    # Find a good place to put drod-nacl
    $ git clone git://github.com/binji/drod-nacl
    $ cd drod-nacl
    $ git submodule init
    $ git submodule update


Building
--------

    First build the necessary libraries. You should only have to do this once.

    $ make ports

    Now build DROD.

    $ make

The output is put in $PWD/out. The data needed for the package is in
$PWD/out/package.


Running
-------

    $ export CHROME_PATH=/path/to/chrome
    $ make run-package
