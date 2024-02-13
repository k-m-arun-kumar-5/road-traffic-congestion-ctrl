Description :
=============
4 way traffic control sequence on one lane present in each way with GO sequence E-W-N-S
direction, with density control. Next sequence direction Go means next direction traffic GO state eg, if in east direction traffic is
in GO, then next direction traffic GO is West. If next sequence directions traffic congestion are in level 0 ie less traffic congestion,
then use normal level 0 Green for next sequence direction traffic GO, eg if in east direction traffic is in GO, then in West GO duration = normal level 0 West GO duration time. If next sequence direction has level 1 traffic congestion ie level 1 congestion more than level 0 congestion, then switching to this sequence will have more green duration time than level 0 green duration time. eg.if in east direction traffic is in GO, then in West GO duration = level 1 West GO
duration time and level 1 West GO duration time > level 0 West GO duration time. If next sequence direction has level 2 traffic congestion ie, level 2 congestion more than level 1 congestion, then switching to this sequence will have more green duration time than level 1 green duration time. eg.if in east direction traffic is in GO, then in West GO duration = level 2 West GO duration time and level 2 West GO duration time > level 1 West GO duration time. We use IR_LEVEL1_SW PRESSED ON to indicate level 1 congestion and IR_LEVEL2_SW PRESSED ON to indicate level 2 congestion for each direction.

CAUTION:
========
Schematics and simulation is done by Proteus CAD. NOT EXPERIMENTED IN REAL TIME ENVIRONMENT.

Purpose :
=========
In all my respective repositories, I just shared my works that I worked as the learning path and practiced, with designed, developed, implemented, simulated and tested, including some projects, assignments, documentations and all other related files and some programming that might not being implement, not being completed, lacks some features or have some bugs. Purpose of all my repositories, if used, can be used for EDUCATIONAL PURPOSE ONLY. It can be used as the open source and freeware. Kindly read the LICENSE.txt for license, terms and conditions about the use of source codes, binaries, documentation and all other files, located in all my repositories. 

