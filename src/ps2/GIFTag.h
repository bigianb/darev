#pragma once

class GIFTag
{
public:
	int nloop;
    bool eop;
    bool pre;
    int prim;
    int flg;
    int nreg;
    int regs[16];

	void parse(const unsigned char* data);
};
