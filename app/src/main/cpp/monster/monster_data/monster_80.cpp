#include "monster_80.h"
unsigned char monster_80[48][48]={
{35,35,35,35,35,35,63,63,63,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,63,63,63,63,63,63,63,63,63,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,45,45,45,45,45,45,63,63,63,63,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,45,45,45,45,45,63,63,63,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,42,42,42,42,35,35,35,45,45,45,45,45,45,63,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,42,42,42,63,63,63,63,63,42,42,42,42,35,35,35,35,35,45,45,45,45,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{63,63,35,35,35,35,35,35,35,35,63,63,63,63,42,42,42,42,35,35,35,35,45,45,63,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,63,63,63,63,63,63,45,63,35,35,35,45,63,42,63,63,42,42,42,35,35,35,45,45,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,45,35,35,35,35,35,35,35,35,63,63,63,63,45,45,63,45,42,63,42,42,42,35,35,35,45,45,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,45,63,63,63,63,63,45,35,35,35,63,63,63,45,45,63,45,42,63,42,42,42,35,35,35,45,45,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,63,63,63,35,35,45,63,63,45,45,63,45,42,42,42,42,35,35,35,45,45,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,45,63,63,45,35,35,63,63,63,63,45,45,42,63,63,45,45,63,42,42,42,35,35,45,45,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,63,45,63,35,35,35,45,63,63,63,45,63,63,45,45,42,42,42,42,35,35,45,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,45,63,63,63,63,45,45,63,45,63,42,42,45,45,63,42,42,35,45,45,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,63,63,45,35,35,35,45,63,63,45,63,63,63,63,45,42,42,42,42,35,45,45,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,63,63,63,63,63,45,63,63,45,63,42,63,42,45,45,42,42,35,45,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,45,35,63,63,45,63,63,45,63,42,63,42,42,35,35,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,63,63,63,35,35,63,45,45,63,63,63,45,45,42,35,35,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,63,63,45,63,63,45,42,63,63,42,42,35,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,63,63,45,45,63,63,63,45,42,35,35,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,35,42,35,63,63,45,42,42,42,35,45,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,45,45,45,42,63,42,42,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,42,42,42,63,45,45,45,42,42,42,42,35,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,42,42,42,42,42,42,42,35,42,42,63,42,63,45,63,63,42,42,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,42,42,42,42,42,42,42,42,63,42,42,42,42,42,42,42,42,42,42,42,63,63,42,42,42,42,42,42,42,42,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{42,42,35,42,35,35,35,63,63,63,45,63,63,42,45,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,63,63,63,42,42,42,42,35,35,35,35,35,35,35,},
{35,35,63,35,42,45,45,45,35,35,35,35,35,35,35,35,63,63,42,63,63,63,42,42,42,42,42,42,42,42,42,42,42,42,42,42,63,63,63,35,42,35,35,35,35,35,35,35,},
{42,35,42,45,45,35,35,35,35,35,35,35,35,35,35,35,35,35,42,42,63,35,35,35,35,35,35,35,35,35,35,35,42,42,42,42,42,42,42,42,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,45,45,35,45,45,45,45,45,45,45,35,35,42,42,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,42,42,42,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,42,42,45,42,42,42,42,42,42,42,35,42,42,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,42,42,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,63,63,63,63,63,63,63,63,63,63,35,42,42,63,35,63,63,63,42,42,42,35,42,35,42,35,42,35,42,42,42,35,42,42,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,45,45,35,35,35,35,35,35,35,35,35,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,45,35,35,63,63,35,63,63,42,42,42,42,42,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,35,63,63,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,63,35,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,63,63,63,63,35,35,42,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,35,35,63,63,63,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,42,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
{35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,},
};