<p align="center"><img src="https://github.com/ksw0518/Turbulence_v4/blob/master/Turbulence_v4/Turbulence.png" width=15% height=15%></p>
<h1 align="center">Turbulence</h1>

<h3 align="center">UCI compatible chess engine written in C++</h3>

# Turbulence
Turbulence is a UCI-compatible chess engine written in C++, developed since August 2024.   
As far as I know, it is the strongest chess engine developed in South Korea.    
(Please let me know if that’s not the case!)   
# How to build
Assuming you have clang++ compiler(has to support c++20), git, and Make, you can easily build the latest dev version using Make.    
1.```git clone https://github.com/ksw0518/Turbulence_v4``` to clone the repository to your computer    
2.cd to the ```Turbulence_v4``` folder    
3.```make```

# strengths
| Version  |      [SP-CC UHO-Top15][spcc]       | [CCRL 40/15][ccrl-4015] | [CCRL Blitz][ccrl-blitz] | [CEGT 40/4][cegt-404] | [CEGT 40/20][cegt-4020] | [MCERL] | [UBC]     |
|:--------:|:----------------------------------:|:-----------------------:|:------------------------:|:---------------------:|:-----------------------:|:-------:|:----------:
|  [0.0.2]   |                 -                  |            -            |           -              |           -           |            -            |    -    |   -       |
|  [0.0.3]   |                 -                  |          2271           |           2192           |           -           |            -            |    -    |   -       |
|  [0.0.4]   |                 -                  |          2747           |           2695           |           -           |            -            |    -    |   2736    |
|  [0.0.5]   |                 -                  |          2960           |           2971           |           -           |            -            |    -    |   2946    |

Development Verison : 3450~3500

**Versions prior to 0.0.6 use HCE(especially PeSTO and bishop pair bonus), while 0.0.6 and later use NNUE.**


# testing
Testing (e.g., SPRT) is conducted using the OpenBench framework:
https://chess.n9x.co/
https://programcidusunur.pythonanywhere.com/index/   



# Credits

- [**Ciekce**](https://github.com/Ciekce), [**Shawn**](https://github.com/xu-shawn), and everyone in the Stockfish discord for helping me implement many features
- [**SwedishChef**](https://github.com/JonathanHallstrom) for providing hardware for datagen and testing
- [**Matt**](https://git.nocturn9x.space/nocturn9x) for letting me join his Openbench instance and providing hardware
- [**Dark Neutrino**](https://github.com/Haxk20) for SEE code



[spcc]: https://www.sp-cc.de/
[ccrl-4015]: https://www.computerchess.org.uk/ccrl/4040/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[ccrl-blitz]: https://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?class=Single-CPU+engines&only_best_in_class=on&num_best_in_class=1&print=Rating+list
[cegt-404]: http://www.cegt.net/40_4_Ratinglist/40_4_single/rangliste.html
[cegt-4020]: http://www.cegt.net/40_40%20Rating%20List/40_40%20All%20Versions/rangliste.html
[mcerl]: https://www.chessengeria.eu/mcerl
[UBC]: https://e4e6.com/
[0.0.2]: https://github.com/ksw0518/Turbulence_v4/releases/tag/v0.0.2
[0.0.3]: https://github.com/ksw0518/Turbulence_v4/releases/tag/v0.0.3
[0.0.4]: https://github.com/ksw0518/Turbulence_v4/releases/tag/v0.0.4
[0.0.5]: https://github.com/ksw0518/Turbulence_v4/releases/tag/v0.0.5-release
