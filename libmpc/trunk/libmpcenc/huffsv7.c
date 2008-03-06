/*
 * Musepack audio compression
 * Copyright (C) 1999-2004 Buschmann/Klemm/Piecha/Wolf
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "libmpcenc.h"

const Huffman_t HuffBands [33] = {
	{1, 1}, {1, 3}, {2, 5}, {2, 6}, {3, 7}, {3, 8}, {4, 8}, {4, 9}, {5, 10}, {6, 11}, {1, 12}, {2, 12}, {3, 12}, {0, 13}, {4, 12}, {5, 12}, {6, 12}, {7, 12}, {8, 12}, {1, 13}, {9, 12}, {10, 12}, {11, 12}, {7, 11}, {8, 11}, {9, 11}, {6, 10}, {7, 10}, {5, 9}, {5, 8}, {3, 6}, {3, 5}, {1, 2}
};

const Huffman_t HuffRes [2] [17] = {
	{
		{1, 1}, {1, 2}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 9}, {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15}, {0, 16}, {1, 16}, {1, 8}, {1, 3}
	}, {
		{1, 2}, {2, 2}, {1, 3}, {1, 5}, {1, 7}, {1, 8}, {1, 10}, {1, 12}, {0, 14}, {1, 14}, {2, 14}, {3, 14}, {1, 11}, {1, 9}, {1, 6}, {1, 4}, {3, 2}
	}
};

const Huffman_t   HuffSCFI_1 [4] = {
	{0, 3}, {1, 3}, {1, 1}, {1, 2}
};

const Huffman_t   HuffSCFI_2 [16] = {
	{1, 6}, {0, 7}, {2, 6}, {3, 6}, {1, 7}, {3, 5}, {4, 5}, {5, 5}, {4, 6}, {6, 5}, {2, 2}, {2, 3}, {5, 6}, {7, 5}, {3, 3}, {3, 2}
};

const Huffman_t   HuffDSCF_1 [64] = {
	{3, 12}, {4, 12}, {5, 12}, {4, 11}, {5, 11}, {6, 11}, {5, 10}, {6, 10}, {7, 10}, {8, 10}, {9, 10}, {7, 9}, {8, 9}, {9, 9}, {10, 9}, {7, 8}, {8, 8}, {9, 8}, {10, 8}, {7, 7}, {8, 7}, {9, 7}, {10, 7}, {6, 6}, {7, 6}, {5, 5}, {4, 4}, {5, 4}, {6, 5}, {6, 4}, {7, 4}, {10, 10}, {8, 4}, {5, 3}, {6, 3}, {7, 3}, {9, 4}, {7, 5}, {8, 6}, {9, 6}, {11, 7}, {11, 8}, {12, 8}, {13, 8}, {11, 9}, {12, 9}, {13, 9}, {11, 10}, {12, 10}, {13, 10}, {7, 11}, {8, 11}, {9, 11}, {6, 12}, {7, 12}, {3, 13}, {4, 13}, {5, 13}, {0, 14}, {1, 14}, {2, 14}, {3, 14}, {4, 14}, {5, 14}
};

const Huffman_t   HuffDSCF_2 [65] = {
	{0, 15}, {3, 14}, {4, 14}, {4, 13}, {5, 13}, {6, 13}, {5, 12}, {6, 12}, {7, 12}, {8, 12}, {7, 11}, {8, 11}, {9, 11}, {10, 11}, {7, 10}, {8, 10}, {9, 10}, {10, 10}, {7, 9}, {8, 9}, {9, 9}, {6, 8}, {7, 8}, {5, 7}, {6, 7}, {4, 6}, {3, 5}, {3, 4}, {4, 4}, {3, 3}, {4, 3}, {5, 3}, {6, 3}, {7, 3}, {5, 4}, {4, 5}, {5, 5}, {5, 6}, {7, 7}, {8, 8}, {9, 8}, {10, 9}, {11, 9}, {11, 10}, {12, 10}, {13, 10}, {11, 11}, {12, 11}, {13, 11}, {9, 12}, {10, 12}, {11, 12}, {12, 12}, {7, 13}, {8, 13}, {9, 13}, {5, 14}, {6, 14}, {7, 14}, {1, 15}, {2, 15}, {3, 15}, {4, 15}, {5, 15}, {13, 12}
};

static const Huffman_t   HuffQ1 [19] = {
	{1, 6}, {1, 4}, {2, 4}, {3, 3}, {4, 3}, {5, 3}, {6, 3}, {7, 3}, {3, 4}, {4, 4}, {5, 4}, {1, 5}, {1, 7}, {1, 8}, {1, 9}, {1, 10}, {1, 11}, {0, 12}, {1, 12}
};

static const Huffman_t   HuffQ2 [2] [5*5*5] = {
	{
		{2, 12}, {3, 11}, {15, 10}, {4, 11}, {0, 13}, {5, 11}, {12, 9}, {18, 8}, {13, 9}, {6, 11}, {7, 11}, {19, 8}, {21, 7}, {20, 8}, {8, 11}, {9, 11}, {14, 9}, {21, 8}, {15, 9}, {10, 11}, {3, 12}, {11, 11}, {16, 10}, {12, 11}, {1, 13}, {13, 11}, {16, 9}, {22, 8}, {17, 9}, {14, 11}, {18, 9}, {15, 6}, {16, 6}, {22, 7}, {19, 9}, {23, 8}, {17, 6}, {8, 4}, {18, 6}, {24, 8}, {20, 9}, {19, 6}, {20, 6}, {23, 7}, {21, 9}, {15, 11}, {22, 9}, {25, 8}, {23, 9}, {16, 11}, {17, 10}, {26, 8}, {24, 7}, {27, 8}, {18, 10}, {28, 8}, {21, 6}, {9, 4}, {22, 6}, {29, 8}, {25, 7}, {10, 4}, {7, 3}, {11, 4}, {26, 7}, {30, 8}, {23, 6}, {12, 4}, {24, 6}, {31, 8}, {19, 10}, {32, 8}, {27, 7}, {33, 8}, {20, 10}, {17, 11}, {24, 9}, {34, 8}, {25, 9}, {18, 11}, {26, 9}, {25, 6}, {26, 6}, {27, 6}, {27, 9}, {35, 8}, {28, 6}, {13, 4}, {29, 6}, {36, 8}, {28, 9}, {28, 7}, {30, 6}, {31, 6}, {29, 9}, {19, 11}, {30, 9}, {37, 8}, {31, 9}, {20, 11}, {2, 13}, {21, 11}, {21, 10}, {22, 11}, {4, 12}, {23, 11}, {32, 9}, {38, 8}, {33, 9}, {24, 11}, {22, 10}, {39, 8}, {29, 7}, {40, 8}, {25, 11}, {26, 11}, {34, 9}, {41, 8}, {35, 9}, {27, 11}, {3, 13}, {28, 11}, {23, 10}, {29, 11}, {5, 12}
	}, {
		{2, 11}, {3, 10}, {15, 9}, {4, 10}, {0, 12}, {5, 10}, {12, 8}, {13, 8}, {14, 8}, {6, 10}, {7, 10}, {15, 8}, {30, 7}, {16, 8}, {16, 9}, {8, 10}, {17, 8}, {18, 8}, {19, 8}, {9, 10}, {3, 11}, {10, 10}, {17, 9}, {11, 10}, {1, 12}, {12, 10}, {20, 8}, {21, 8}, {22, 8}, {13, 10}, {23, 8}, {18, 6}, {14, 5}, {19, 6}, {24, 8}, {25, 8}, {20, 6}, {15, 5}, {16, 5}, {26, 8}, {27, 8}, {21, 6}, {17, 5}, {22, 6}, {28, 8}, {14, 10}, {29, 8}, {30, 8}, {31, 8}, {15, 10}, {18, 9}, {32, 8}, {31, 7}, {33, 8}, {19, 9}, {34, 8}, {18, 5}, {19, 5}, {20, 5}, {35, 8}, {32, 7}, {21, 5}, {15, 4}, {22, 5}, {33, 7}, {36, 8}, {23, 5}, {24, 5}, {25, 5}, {37, 8}, {20, 9}, {38, 8}, {34, 7}, {39, 8}, {21, 9}, {16, 10}, {40, 8}, {41, 8}, {42, 8}, {17, 10}, {43, 8}, {23, 6}, {26, 5}, {24, 6}, {44, 8}, {45, 8}, {27, 5}, {28, 5}, {25, 6}, {46, 8}, {47, 8}, {26, 6}, {29, 5}, {27, 6}, {48, 8}, {18, 10}, {49, 8}, {50, 8}, {51, 8}, {19, 10}, {2, 12}, {20, 10}, {21, 10}, {22, 10}, {4, 11}, {23, 10}, {52, 8}, {53, 8}, {54, 8}, {24, 10}, {22, 9}, {55, 8}, {35, 7}, {56, 8}, {25, 10}, {26, 10}, {57, 8}, {58, 8}, {59, 8}, {27, 10}, {3, 12}, {28, 10}, {23, 9}, {29, 10}, {5, 11}
	}
};

static const Huffman_t   HuffQ3 [49] = {
	{0, 9}, {2, 8}, {5, 7}, {6, 7}, {7, 7}, {3, 8}, {1, 9}, {4, 8}, {9, 6}, {10, 6}, {10, 5}, {11, 6}, {8, 7}, {5, 8}, {9, 7}, {12, 6}, {8, 4}, {9, 4}, {11, 5}, {13, 6}, {10, 7}, {11, 7}, {12, 5}, {10, 4}, {7, 3}, {11, 4}, {13, 5}, {12, 7}, {13, 7}, {14, 6}, {14, 5}, {12, 4}, {13, 4}, {15, 6}, {14, 7}, {6, 8}, {16, 6}, {17, 6}, {15, 5}, {18, 6}, {19, 6}, {7, 8}, {2, 9}, {8, 8}, {15, 7}, {16, 7}, {17, 7}, {9, 8}, {3, 9}
};

static const Huffman_t   HuffQ4 [81] = {
	{0, 10}, {2, 9}, {5, 8}, {12, 7}, {13, 7}, {6, 8}, {7, 8}, {3, 9}, {1, 10}, {4, 9}, {8, 8}, {14, 7}, {13, 6}, {14, 6}, {15, 6}, {15, 7}, {9, 8}, {5, 9}, {10, 8}, {16, 7}, {16, 6}, {17, 6}, {18, 5}, {18, 6}, {19, 6}, {17, 7}, {11, 8}, {12, 8}, {20, 6}, {21, 6}, {19, 5}, {20, 5}, {21, 5}, {22, 6}, {23, 6}, {13, 8}, {18, 7}, {24, 6}, {22, 5}, {23, 5}, {15, 4}, {24, 5}, {25, 5}, {25, 6}, {19, 7}, {14, 8}, {26, 6}, {27, 6}, {26, 5}, {27, 5}, {28, 5}, {28, 6}, {29, 6}, {15, 8}, {16, 8}, {20, 7}, {30, 6}, {31, 6}, {29, 5}, {32, 6}, {33, 6}, {21, 7}, {17, 8}, {6, 9}, {18, 8}, {22, 7}, {23, 7}, {34, 6}, {35, 6}, {24, 7}, {19, 8}, {7, 9}, {2, 10}, {8, 9}, {20, 8}, {21, 8}, {25, 7}, {22, 8}, {23, 8}, {9, 9}, {3, 10}
};

static const Huffman_t   HuffQ5 [2] [15] = {
	{
		{0, 7}, {1, 7}, {2, 6}, {2, 5}, {2, 4}, {2, 3}, {3, 3}, {3, 2}, {4, 3}, {5, 3}, {3, 4}, {3, 5}, {3, 6}, {2, 7}, {3, 7}
	}, {
		{0, 6}, {1, 6}, {2, 5}, {2, 4}, {3, 4}, {3, 3}, {4, 3}, {5, 3}, {6, 3}, {7, 3}, {4, 4}, {5, 4}, {3, 5}, {2, 6}, {3, 6}
	}
};

static const Huffman_t   HuffQ6 [2] [31] = {
	{
		{0, 9}, {1, 9}, {2, 9}, {3, 9}, {4, 8}, {5, 8}, {4, 7}, {3, 6}, {4, 6}, {5, 6}, {5, 5}, {6, 5}, {4, 4}, {5, 4}, {4, 3}, {3, 2}, {5, 3}, {6, 4}, {7, 4}, {7, 5}, {6, 6}, {7, 6}, {8, 6}, {9, 6}, {5, 7}, {6, 8}, {7, 8}, {4, 9}, {5, 9}, {6, 9}, {7, 9}
	}, {
		{0, 8}, {1, 8}, {2, 7}, {3, 7}, {4, 7}, {4, 6}, {5, 6}, {4, 5}, {5, 5}, {6, 5}, {5, 4}, {6, 4}, {7, 4}, {8, 4}, {9, 4}, {10, 4}, {11, 4}, {12, 4}, {13, 4}, {14, 4}, {15, 4}, {7, 5}, {8, 5}, {9, 5}, {6, 6}, {7, 6}, {5, 7}, {6, 7}, {7, 7}, {2, 8}, {3, 8}
	}
};

static const Huffman_t   HuffQ7 [2] [63] = {
	{
		{0, 10}, {1, 10}, {2, 10}, {8, 9}, {9, 9}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {7, 10}, {10, 9}, {11, 9}, {12, 9}, {13, 9}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {10, 7}, {11, 7}, {12, 7}, {13, 7}, {14, 7}, {10, 6}, {11, 6}, {12, 6}, {8, 5}, {9, 5}, {6, 4}, {4, 3}, {3, 2}, {5, 3}, {7, 4}, {10, 5}, {11, 5}, {13, 6}, {14, 6}, {15, 6}, {15, 7}, {16, 7}, {17, 7}, {18, 7}, {15, 8}, {19, 7}, {16, 8}, {17, 8}, {18, 8}, {19, 8}, {14, 9}, {15, 9}, {16, 9}, {17, 9}, {8, 10}, {9, 10}, {10, 10}, {11, 10}, {12, 10}, {18, 9}, {19, 9}, {13, 10}, {14, 10}, {15, 10}
	}, {
		{0, 9}, {1, 9}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8}, {8, 7}, {9, 7}, {10, 7}, {11, 7}, {12, 7}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {15, 6}, {16, 6}, {12, 5}, {13, 5}, {14, 5}, {15, 5}, {16, 5}, {17, 5}, {18, 5}, {19, 5}, {20, 5}, {21, 5}, {22, 5}, {23, 5}, {24, 5}, {25, 5}, {26, 5}, {27, 5}, {28, 5}, {29, 5}, {30, 5}, {31, 5}, {17, 6}, {18, 6}, {19, 6}, {20, 6}, {21, 6}, {22, 6}, {23, 6}, {13, 7}, {14, 7}, {15, 7}, {16, 7}, {17, 7}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {15, 8}, {2, 9}, {3, 9}
	}
};

static const Huffman_t   HuffQ8 [2] [127] = {
	{
		{3, 11}, {4, 11}, {10, 10}, {11, 10}, {12, 10}, {13, 10}, {14, 10}, {26, 9}, {15, 10}, {27, 9}, {16, 10}, {0, 12}, {1, 12}, {5, 11}, {6, 11}, {7, 11}, {8, 11}, {9, 11}, {10, 11}, {11, 11}, {17, 10}, {12, 11}, {18, 10}, {19, 10}, {20, 10}, {21, 10}, {22, 10}, {23, 10}, {24, 10}, {25, 10}, {28, 9}, {26, 10}, {27, 10}, {28, 10}, {29, 10}, {30, 10}, {29, 9}, {30, 9}, {31, 9}, {32, 9}, {33, 9}, {34, 9}, {35, 9}, {36, 9}, {25, 8}, {37, 9}, {26, 8}, {27, 8}, {28, 8}, {29, 8}, {30, 8}, {31, 8}, {20, 7}, {21, 7}, {22, 7}, {23, 7}, {14, 6}, {15, 6}, {16, 6}, {17, 6}, {11, 5}, {7, 4}, {4, 3}, {3, 2}, {5, 3}, {12, 5}, {13, 5}, {18, 6}, {19, 6}, {20, 6}, {21, 6}, {24, 7}, {25, 7}, {26, 7}, {27, 7}, {32, 8}, {33, 8}, {34, 8}, {35, 8}, {36, 8}, {37, 8}, {38, 8}, {39, 8}, {38, 9}, {39, 9}, {40, 9}, {41, 9}, {42, 9}, {43, 9}, {44, 9}, {45, 9}, {46, 9}, {31, 10}, {32, 10}, {47, 9}, {33, 10}, {34, 10}, {35, 10}, {36, 10}, {37, 10}, {38, 10}, {39, 10}, {40, 10}, {41, 10}, {13, 11}, {14, 11}, {42, 10}, {15, 11}, {16, 11}, {17, 11}, {18, 11}, {2, 12}, {19, 11}, {3, 12}, {4, 12}, {5, 12}, {43, 10}, {44, 10}, {48, 9}, {49, 9}, {45, 10}, {46, 10}, {47, 10}, {48, 10}, {49, 10}, {50, 10}, {51, 10}
	}, {
		{0, 9}, {1, 9}, {2, 9}, {3, 9}, {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8}, {9, 8}, {10, 8}, {11, 8}, {12, 8}, {13, 8}, {14, 8}, {15, 8}, {16, 8}, {17, 8}, {18, 8}, {19, 8}, {20, 8}, {21, 8}, {21, 7}, {22, 7}, {23, 7}, {24, 7}, {25, 7}, {26, 7}, {27, 7}, {28, 7}, {29, 7}, {30, 7}, {31, 7}, {32, 7}, {33, 7}, {34, 7}, {35, 7}, {36, 7}, {37, 7}, {38, 7}, {39, 7}, {40, 7}, {41, 7}, {42, 7}, {43, 7}, {44, 7}, {45, 7}, {46, 7}, {47, 7}, {48, 7}, {38, 6}, {39, 6}, {40, 6}, {41, 6}, {42, 6}, {43, 6}, {44, 6}, {45, 6}, {46, 6}, {47, 6}, {48, 6}, {49, 6}, {50, 6}, {51, 6}, {52, 6}, {53, 6}, {54, 6}, {55, 6}, {56, 6}, {57, 6}, {58, 6}, {59, 6}, {60, 6}, {61, 6}, {62, 6}, {49, 7}, {63, 6}, {50, 7}, {51, 7}, {52, 7}, {53, 7}, {54, 7}, {55, 7}, {56, 7}, {57, 7}, {58, 7}, {59, 7}, {60, 7}, {61, 7}, {62, 7}, {63, 7}, {64, 7}, {65, 7}, {66, 7}, {67, 7}, {68, 7}, {69, 7}, {70, 7}, {71, 7}, {72, 7}, {73, 7}, {74, 7}, {75, 7}, {22, 8}, {23, 8}, {24, 8}, {25, 8}, {26, 8}, {27, 8}, {28, 8}, {29, 8}, {30, 8}, {31, 8}, {32, 8}, {33, 8}, {34, 8}, {35, 8}, {36, 8}, {37, 8}, {38, 8}, {39, 8}, {40, 8}, {41, 8}, {4, 9}, {5, 9}, {6, 9}, {7, 9}
	}
};

const Huffman_t   HuffQ9up [256] = {
	{1, 10}, {2, 10}, {3, 10}, {4, 10}, {5, 10}, {5, 9}, {6, 9}, {7, 9}, {8, 9}, {9, 9}, {10, 9}, {11, 9}, {12, 9}, {13, 9}, {14, 9}, {15, 9}, {16, 9}, {17, 9}, {18, 9}, {38, 8}, {39, 8}, {19, 9}, {20, 9}, {21, 9}, {22, 9}, {23, 9}, {24, 9}, {25, 9}, {26, 9}, {27, 9}, {28, 9}, {29, 9}, {30, 9}, {31, 9}, {32, 9}, {33, 9}, {34, 9}, {35, 9}, {36, 9}, {37, 9}, {40, 8}, {38, 9}, {41, 8}, {42, 8}, {43, 8}, {44, 8}, {45, 8}, {46, 8}, {47, 8}, {48, 8}, {49, 8}, {50, 8}, {51, 8}, {52, 8}, {53, 8}, {54, 8}, {55, 8}, {56, 8}, {57, 8}, {58, 8}, {59, 8}, {60, 8}, {61, 8}, {62, 8}, {63, 8}, {64, 8}, {65, 8}, {66, 8}, {67, 8}, {68, 8}, {69, 8}, {70, 8}, {71, 8}, {72, 8}, {73, 8}, {74, 8}, {75, 8}, {76, 8}, {77, 8}, {78, 8}, {79, 8}, {80, 8}, {81, 8}, {82, 8}, {83, 8}, {84, 8}, {85, 8}, {86, 8}, {87, 8}, {88, 8}, {89, 8}, {90, 8}, {91, 8}, {92, 8}, {93, 8}, {94, 8}, {95, 8}, {96, 8}, {97, 8}, {98, 8}, {99, 8}, {100, 8}, {101, 8}, {102, 8}, {103, 8}, {104, 8}, {105, 8}, {106, 8}, {86, 7}, {87, 7}, {88, 7}, {89, 7}, {90, 7}, {91, 7}, {92, 7}, {93, 7}, {94, 7}, {95, 7}, {96, 7}, {97, 7}, {98, 7}, {99, 7}, {100, 7}, {101, 7}, {102, 7}, {103, 7}, {104, 7}, {62, 6}, {63, 6}, {105, 7}, {106, 7}, {107, 7}, {108, 7}, {109, 7}, {110, 7}, {111, 7}, {112, 7}, {113, 7}, {114, 7}, {115, 7}, {116, 7}, {117, 7}, {118, 7}, {119, 7}, {120, 7}, {121, 7}, {122, 7}, {107, 8}, {123, 7}, {108, 8}, {109, 8}, {110, 8}, {111, 8}, {112, 8}, {113, 8}, {114, 8}, {115, 8}, {116, 8}, {117, 8}, {118, 8}, {119, 8}, {120, 8}, {121, 8}, {122, 8}, {123, 8}, {124, 8}, {125, 8}, {126, 8}, {127, 8}, {128, 8}, {129, 8}, {130, 8}, {131, 8}, {132, 8}, {133, 8}, {134, 8}, {135, 8}, {136, 8}, {137, 8}, {138, 8}, {139, 8}, {140, 8}, {141, 8}, {142, 8}, {143, 8}, {144, 8}, {145, 8}, {146, 8}, {147, 8}, {148, 8}, {149, 8}, {150, 8}, {151, 8}, {152, 8}, {153, 8}, {154, 8}, {155, 8}, {156, 8}, {157, 8}, {158, 8}, {159, 8}, {160, 8}, {161, 8}, {162, 8}, {163, 8}, {164, 8}, {165, 8}, {166, 8}, {167, 8}, {168, 8}, {169, 8}, {170, 8}, {171, 8}, {39, 9}, {40, 9}, {41, 9}, {42, 9}, {43, 9}, {44, 9}, {45, 9}, {46, 9}, {47, 9}, {48, 9}, {49, 9}, {50, 9}, {51, 9}, {52, 9}, {53, 9}, {54, 9}, {55, 9}, {56, 9}, {57, 9}, {58, 9}, {59, 9}, {60, 9}, {61, 9}, {62, 9}, {63, 9}, {64, 9}, {65, 9}, {66, 9}, {67, 9}, {68, 9}, {69, 9}, {70, 9}, {71, 9}, {72, 9}, {73, 9}, {74, 9}, {75, 9}, {6, 10}, {7, 10}, {8, 10}, {9, 10}, {0, 11}, {1, 11}
};

Huffman_t const * const HuffQ [2] [8] = {
	{ HuffQ1, HuffQ2[0], HuffQ3, HuffQ4, HuffQ5[0], HuffQ6[0], HuffQ7[0], HuffQ8[0] },
	{ HuffQ1, HuffQ2[1], HuffQ3, HuffQ4, HuffQ5[1], HuffQ6[1], HuffQ7[1], HuffQ8[1] }
};

/* end of huffsv7.c */
