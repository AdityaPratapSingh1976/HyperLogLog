# HyperLogLog
This is Implementation of [HyperLogLog](https://static.googleusercontent.com/media/research.google.com/en//pubs/archive/40671.pdf) data-structure in C++ is used to estimate cardinality 
of objects of any data-type.
This is done via serialiazation of objects to simple byte-array which are then hashed into 64bit keys.
## Pre-requisites 
boost library is used for serialization which can be downloaded from
 ```
https://www.boost.org/users/download/
 ```
xxHash is used for hashing which can be downloaded from
 ```
https://github.com/Cyan4973/xxHash
 ```
It is used for it's high bandwith but any other good hash function can be used which can give or can be modified to give a 64-bit hash key

## Usage 
It's a header-only implementation and only require including "hll.h" file.
```
#include<iostream>
#include<string>
#include<fstream>
#include "hll.h"
using namespace std;



void solve(){
    ifstream ifile;
    string file_path;
    printf("Enter File Path : ");
    cin >> file_path;
    ifile.open(file_path);

    string line;
    hyperloglog<string> hl(11);

    while (getline(ifile,line)){
        hl.add_element(line);
    }

    double raw_estimate = hl.get_cardinality();
    cout << "Estimate : " <<  raw_estimate<<endl;
}



int main() {
    cout << setprecision(15);
    solve();
    cerr << "Time elapsed:" << 1.0 * clock() / CLOCKS_PER_SEC << " s.\n";
    return 0;
}

```
## License

MIT License

Copyright (c) [2025] [Aditya Pratap Singh]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

