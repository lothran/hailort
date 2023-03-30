<p align="left">
  <img src=".hailort.png" />
</p>


# HailoRT #

HailoRT is a lightweight, production-grade runtime library that runs on the host processor and provides a robust
user-space runtime library (the HailoRT Library) with intuitive APIs in C/C++ for optimized performance

HailoRT consists of the following main components:
- HailoRT Library.
- HailoRT CLI - a command line application used to control the Hailo device, run inferences, collect statistics and device events, etc.
- [**HailoRT PCIe Driver**](https://github.com/hailo-ai/hailort-drivers) - the device driver used to manage the Hailo device, communicate with the device,
and transfer data to/from the device; it includes the Hailo-8 firmware that runs on the Hailo device and manages its boot and control.
- pyHailoRT - HailoRT Python API, which wraps the runtime library.
- HailoRT GStreamer element (HailoNet).

HailoRT supports Linux and Windows, and it can be compiled from sources to be integrated with various x86 and ARM processors.

## Usage

See [**hailo.ai developer zone documentation**](https://hailo.ai/developer-zone/documentation/hailort/latest/) (registration is required for  full documentation access).

For compilation instructions, see  [**Compiling HailoRT from Sources**](https://hailo.ai/developer-zone/documentation/hailort/latest/?sp_referrer=install/install.html#compiling-from-sources).

For HailoRT API examples - see [**HailoRT examples**](https://github.com/hailo-ai/hailort/tree/master/hailort/libhailort/examples).

## Changelog

See [**hailo.ai developer zone - HailoRT changelog**](https://hailo.ai/developer-zone/documentation/hailort/latest/?sp_referrer=changelog/changelog.html) (registration required).

## Licenses

HailoRT uses 2 licenses:
- libhailort, pyhailort & hailortcli - distributed under the [**MIT license**](https://opensource.org/licenses/MIT)
- hailonet (GStreamer plugin)  -  distributed under the [**LGPL 2.1 license**](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt)

## Contact

Contact information and support is available at [**hailo.ai**](https://hailo.ai/contact-us/).

## About Hailo-8™

Hailo-8 is a deep learning processor for edge devices. The Hailo-8 provides groundbraking efficiency for neural network deployment.
The Hailo-8 edge AI processor, featuring up to 26 tera-operations per second (TOPS), significantly outperforms all other edge processors.
Hailo-8 is available in various form-factors, including the Hailo-8 M.2 Module.

The Hailo-8 AI processor is designed to fit into a multitude of smart machines and devices, for a wide variety of sectors including Automotive, Smart Cities, Industry 4.0,
Retail and Smart Homes.

For more information, please visit [**hailo.ai**](https://hailo.ai/).
