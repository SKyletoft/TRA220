#!/usr/bin/env python3

import numpy as np
import plotly.graph_objects as go

data = np.loadtxt('/tmp/poisson.txt')
width, height = int(data[0, 0]), int(data[0, 1])
x, y, z = data[1:, 0], data[1:, 1], data[1:, 2]
X = x.reshape(width, height)
Y = y.reshape(width, height)
Z = z.reshape(width, height)

fig = go.Figure(data=[go.Surface(x=X, y=Y, z=Z)])
fig.show()
