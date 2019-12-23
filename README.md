# Nothing

## Introduction

Nothing is a Windows desktop search engine, just like [Everything](https://en.wikipedia.org/wiki/Everything_(software))

## Document

Click [here](https://shimo.im/docs/cVwgyQPhj6vGThP9/) for our online Document.

## Environments

This software is running on Windows 10(1903 or newer). To build from source, you need to provide the following tools:

* Visual Studio 2019
* Qt 5.13
* Python 3
* Python package: textract = 1.6.3

For more information, see **Instruction**

## Instruction

Download source code on master branch. Before running Project ***frontend***,  you should configure solution properties as follows:

* **Add [Pybind11](https://github.com/pybind/pybind11) dependency:** Download the source file, add `path/to/Pybind11/include` to project ***backend*** `Additional Include Directories`

* **Add python dependency:** Install python3.*(3.5 or newer is recommended) on your Win10, add `path/to/python/include` to project ***backend*** `Additional Include Directories`, add `path/to/python/libs` to project ***backend*** `Additional Library Directories`

Last but not least, **please run this program as administrator**.

## Authors

**纳鑫**
* 学校：清华大学软件学院
* Email：naxinlegend@outlook.com

**王泽宇**
* 学校：清华大学软件学院
* Email：ycdfwzy@outlook.com
