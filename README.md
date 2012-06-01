# Robust Parameter-free Coin Detector

This is the final project of my Image Processing and Modeling in ZJU.

Partner:

* [Yu Du](answeror@gmail.com)
* [Jianqing Chen](qingmarch@gmail.com)

## Snapshots

![Raw](https://raw.github.com/Answeror/RPCD/master/images/snapshot.raw.png)

![Result](https://raw.github.com/Answeror/RPCD/master/images/snapshot.result.png)

![Overlapped](https://raw.github.com/Answeror/RPCD/master/images/snapshot.overlapped.png)

## Project structure

* doc: word and pdf report
* bin: exe and dll
* misc/coin: test data

## How to build

This proejct depents on:

* Win7 x64
* VS2010 SP1
* CMake 2.8.5
* OpenCV 2.3.1
* Qt 4.7.2
* Boost 1.47.0
* Qwt 6.0.1
* [CML 1.0.3](http://cmldev.net/)
* [OvenToBoost](http://github.com/faithandbrave/OvenToBoost)
* [ACMake](http://github.com/Answeror/ACMake)
* [Ans](http://github.com/Answeror/ans)

CML, ACMake, Ans and OvenToBoost is used as submodule.

If you have Git, VS2010, CMake, OpenCV, Qt, Boost, and Qwt installed and following environment variables properly set:

* QTDIR
* QWT_HOME
* OpenCV_DIR
* BOOST_ROOT

Then you can open `Visual Studio Command Prompt (2010)` and use these commands to build this project:

```bash
git clone git@github.com:Answeror/RPCD.git rpcd
cd rpcd
git submodule init
git submodule update
build
```

The `final.exe` will be generated under `build` folder.

# Compare

You can use test data under `misc/coin` to compare with <http://ceng.anadolu.edu.tr/cv/EDCircles/>.
