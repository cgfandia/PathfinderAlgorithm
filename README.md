# FPGA Pathfinder Algorithm

## About Algorithm

**_Pathfinder Algorithm_** composed of two parts: a _signal router_, which
routes one signal at a time using a shortest-path algorithm,
and a _global router_, which calls the signal router to route all
signals, adjusting the resource costs in order to achieve a
complete routing. The signal router uses a breadth-first
search to find the shortest path given a congestion cost and
delay for each routing resource. The global router
dynamically adjusts the congestion penalty of each routing
resource based on the demands signals place on that
resource. During the first iteration of the global router there
is no cost for sharing routing resources and individual
routing resources may be used by more than one signal.
However, during subsequent iterations the penalty is
gradually increased so that signals in effect negotiate for
resources. Signals may use shared resources that are in
high demand if all alternative routes utilize resources in
even higher demand; other signals will tend to spread out
and use resources in lower demand. The global router
reroutes signals using the signal router until no more
resources are shared. The use of a cost function that
gradually increases the penalty for sharing is a significant
departure from Nair’s algorithm, which assigns a cost of
infinity to resources whose capacity is exceeded.

## Realization

_Realization_ of algorithm based on _.place_ and _.net_ files from _"FPGA Place-and-Route Challenge"_ from _University of Toronto_. To visualize routing used _OpenGL_. To read more: [docs](/docs) or [here](http://www.eecg.toronto.edu/~vaughn/challenge/challenge.html)

## How to build
###In Linux
Run ```./build.sh```

**_Required_**:

```
cmake
libxmu-dev libxi-dev
freeglut3 freeglut3-dev
```

###In Windows
Use _CMake_ application to create _MVS_ project and then compile

## Usage
```./PATHFINDER .place-file .net-file Fvp Fvh Iterations-count```