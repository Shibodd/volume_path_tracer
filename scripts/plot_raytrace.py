import csv
import numpy as np



import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def register_handler(registry):
  def deco(func):
    registry[func.__name__] = func
    return func
  return deco

class EventManager:
  handlers = {}

  def __init__(self):
    self.points = []
    self.rays = []
  
  def on_event(self, event, args):
    self.handlers[event](self, *args)

  def show(self):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    points = np.array(self.points)
    points = points.reshape((-1,3))

    rays = np.array(self.rays)
    rays = rays.reshape((-1,6))

    ray_pts = np.vstack([rays[:,:3], np.array(rays[-1,:3] + rays[-1,3:] * 3)])

    ax.plot(ray_pts[:,0], ray_pts[:,1], ray_pts[:,2])
    # ax.scatter(rays[:,0] + rays[:,3] * 2, rays[:,1] + rays[:,4] * 2, rays[:,2] + rays[:,5] * 2)
    ax.scatter(points[:,0], points[:,1], points[:,2])
    plt.show()

    self.points = []
    self.rays = []
  
  @register_handler(handlers)
  def new_ray(self, ox, oy, oz, dx, dy, dz):
    print("new ray")
    if len(self.rays) > 0:
      self.show()
    self.rays.append(np.array([ox, oy, oz, dx, dy, dz]))

  @register_handler(handlers)
  def sampled_point(self, px, py, pz, d):
    print("Density", d)
    self.points.append(np.array([px, py, pz]))

  @register_handler(handlers)
  def null(self):
    print("Null")

  @register_handler(handlers)
  def scatter_terminated(self):
    print("Maxdepth reached")
    self.show()

  @register_handler(handlers)
  def scatter(self, ox, oy, oz, dx, dy, dz):
    print("Scatter")
    self.rays.append(np.array([ox, oy, oz, dx, dy, dz]))

  @register_handler(handlers)
  def absorbed(self):
    print("Absorbed")
    self.show()

mgr = EventManager()

with open("log.csv", "rt") as f:
  r = csv.reader(f)
  for row in r:
    mgr.on_event(row[0], [float(x) for x in row[1:]])