#pragma once
#include "Board.h"
#include <string>

void init_tables();
int Evaluate(Board& board);

void print_tables();
void LoadNetwork(const std::string& filepath);