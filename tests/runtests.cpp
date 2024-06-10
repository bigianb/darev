#include "runtests.h"

#if defined(__mips__)
#include <sifrpc.h>
#endif

int main(int argc, char* argv[])
{
#if defined(__mips__)
    SifInitRpc(0);
#endif

    printf("Starting tests\n");
    fflush(stdout);

    bool okay = true;

	okay &= runDlistTests();
	okay &= runGSAllocTests();
    if (okay){
        printf("All good\n");
    } else {
        printf("Tests failed\n");
    }

    fflush(stdout);

#if defined(__mips__)
    while(true){}
#endif

	return okay ? 0 : -1;
}
