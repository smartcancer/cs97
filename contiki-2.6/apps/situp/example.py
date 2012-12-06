#!/usr/bin/env python
"""
Demonstrates the plot responding to data updates while remaining responsive
to user interaction.  Panning and Zooming work as described in simple_line.py

There is a timer set which modifies the data that is passed to the plot.
Since the axes and grids automatically determine their range from the
dataset, they rescale each time the data changes.  This gives the zooming
in and out behavior.  As soon as the user interacts with the plot by panning
or manually zooming, the bounds of the axes are no longer "auto", and it
becomes more apparent that the plot's data is actually changing as a
function of time.

Original inspiration for this demo from Bas van Dijk.
"""

from pylab import *
import time

ion()

tstart = time.time()               # for profiling
x = arange(0,2*pi,0.01)            # x-array
line, = plot(x,sin(x))
for i in arange(1,200):
    line.set_ydata(sin(x+i/10.0))  # update the data
    draw()                         # redraw the canvas

print 'FPS:' , 200/(time.time()-tstart)
