#!/usr/bin/env python3

import numpy as np
import plotly.graph_objects as go

data = np.loadtxt('/tmp/poisson.txt')
x, y, z = data[:, 0], data[:, 1], data[:, 2]
X = x.reshape(50, 50)
Y = y.reshape(50, 50)
Z = z.reshape(50, 50)

fig = go.Figure(data=[go.Surface(x=X, y=Y, z=Z)])
fig.show()
