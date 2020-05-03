Evolution of Trust
==================

In one of classical games from the game theory, two players independently decide
if they put a coin into a machine. Once a player puts a coin, his opponent gets
three coins. A naive strategy would not pay at all, in order reduce costs and
to maximize the profit. However, such a cheating strategy may discourage the
opponent to pay in the next turns. Therefore, to get best scores, players must
cooperate and trust each other.

This program simulates, how the trust evolves. Each strategy is described by a
probabilistic finite state automaton, where each state contains probability on
which the player pays, and each transition is labeled by the decision of the
opponent. Such automata are placed on 2D plane (with torus topology), and on
each simulation step they plays several turns of the game with their neighbors.
Automata with the locally worst scores are replaced by new ones, by mutating
survivors from the neighborhood.

The program was inspired by Nicky Case's
[Evolution of Trust](https://ncase.me/trust/), found on the Web.

Please mail comments and bug reports to
Piotr Polesiuk <ppolesiuk@cs.uni.wroc.pl>

Compiling
---------

Just type
```
$ make
```

to build the project

Usage
-----

Program can be configured many ways by command line options. To see full list
of program option, run it with `--help` option
```
$ ./trust --help
```
Most interesting options are shown on the following example and described
below.
```
$ ./trust       \
    -o stat.dat \
    -i img_     \
    -x a_       \
    -b 64x64    \
    -m 0.03     \
    -d -a -A    \
    -s 20       \
    -t 50       \
```

- `-o stat.dat` specifies the name of the file, where the average final score
  is recorded over time. Those data can be viewed by `show_stat.gp`, even while
  the program is running.

- `-i img_` specifies names of files, where the map of final scores is drawn as
  a PNG image (automatons are placed on 2D plane). In this example such images
  would have names `img_0.png`, `img_10.png`, `img_20.png`, ... (the number in
  the file name is the number of simulation steps). You can generate a movie
  from those files using, e.g., `ffmpeg`:
  ```
  $ ffmpeg -f image2 -i img_%d0.png movie.mp4
  ```

- `-x a_`, specifies names of files, where randomly picked automata will be
  saved as Graphviz script. In this example they would be saves as `a_0.gv`,
  `a_200.gv`, `a_400.gv`, ... `draw_automaton.sh` script can transform them to
  PNG images using Graphviz.

- `-b 64x64` specifies the size of the plane, where automata are placed,
  and therefore the number of automata in the simulation.

- `-m 0.03` specifies how often automata make mistakes, i.e., how often the
  effective outcome of their move is different that their decision (3% in this
  example). Note that if automaton decides to pay, but makes mistake, its
  opponent sees it as cheating.

- `-d -a -A` flags specifies capabilities of automata. `-d` stands for 
  *deterministic*, which means that probability of paying or cheating is always
  0 or 1. The `-a` flag makes automata aware of their decisions, i.e. they can
  go to different states depending on the decision that they made (pay or
  cheat). In such a case, transitions are labeled by two numbers: first one
  is the automaton decision (not necessarily move, because of mistakes), and
  the second is the opponent move. Of course, `-a` flag does not change
  anything for deterministic automata.
  Finally, `-A` makes automata aware of own mistakes. They have extra
  transitions (labeled with `#`) that describes what to do if they make a
  mistake.

- `-s 20` specifies the number of automata states (here, 20). Effective number
  of states is usually smaller, because not all states are reachable.

- `-t 50` specifies the number of turns in each game.

The programs backups its state to `world` file from time to time (every 1000
simulation steps by default) or when it gets `SIGINT` signal. So in case of
e.g., power failure you can continue from the backup. In order to do so, pass
`--continue` option to the program (other options are ignored in such a case).

Have fun!
