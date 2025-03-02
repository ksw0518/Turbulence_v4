<img src="https://github.com/ksw0518/Turbulence_v4/blob/master/Turbulence_v4/Turbulence.png" width=15% height=15%>

# Turbulence
Turbulence is a UCI compatible chess engine written in C++
developed since 08/20/2024   

# How to build
Assuming you have clang++ compiler(has to support c++20), git and Make, you can easily build the latest dev version using Make.    
1.```git clone https://github.com/ksw0518/Turbulence_v4``` to clone the repository to your computer    
2.cd to the ```Turbulence_v4``` folder    
3.```make```

# estimated strength
rating with * means not tested on CCRL blitz list    
v0.0.2 : 1700*    
v0.0.3 : 2266     
v0.0.4 : 2734     
v0.0.5 : 2980*    
(next releasing coming soon)    
>>>>>>>>>>>>>>>>>>>>>>>>>>>
dev    : 2900~3000*    
# testing
testings like SPRT was done on OpenBench framework.   
https://chess.n9x.co/
https://programcidusunur.pythonanywhere.com/index/   

# additional info
The goal is to go 3000 with pesto + bishop pair bonus eval

# Credits
Special thanks to Matt(author of Heimdall), SwedishChef(author of Pawnochinno), Quinn(author of Prelude), and Eren(author of Potential) for providing cores for testing.  
Thanks to everyone in SF discord and EP discord, which helped me understand many chess engine related techniques.   
Also DarkNeutro(author of Quanticade) for Static Exchange Evaluation source code.   
Maksim Korzh for good youtube series of chess engine tutorial(BBC chess engine), which I used for magic number generation.   
and finally Sebastian lague for creating a nice video of developing chess engine.   
