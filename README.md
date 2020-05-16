# High-Level Design projects
 
This repository has all the documentation and codes for the High-Level Design projects.

## Project description

The project is a video module which receives a VGA signal, process the stream transforming the signal passing a series of filters and transmits the video signal through VGA again.

Basically, this project is composed of the following parts:

1. **VGA decoder**: Receives the VGA signal from an Analog-to-Digital converter (ADC) and transforms it into either a digital video stream or stores the image into a memory.

2. **Video filter**: There are a couple of ideas:

- Simply grayscaling the image (easy way and supports streaming)
- Detecting borders using either Canny or using Difference of Gaussians (DoG) (difficult because it needs a convolution)

3. **VGA encoder**: Retrieves the image from the stream (or memory) and streams it to a Digital-to-Analog converter, which will transform the digital signals into analog signals.

## Requirements

* G++ compiler
* SystemC libraries (install using ´sudo apt install libsystemc-dev´)
* GTKwave for vcd test waveform visualization

## Compilation

```bash
cd src
make
```

> You may need to have the SystemC library installed on your computer and modify 
the construction system to get it work. 

## Disclaimer

This code is intended for learning purposes. We are not responsible for its misuse.

I Quarter - 2019
