#include "runtests.h"


int main(int argc, char* argv[])
{
    printf("Starting tests\n");

    bool okay = true;

	okay &= runDlistTests();
	
    if (okay){
        printf("All good\n");
    } else {
        printf("Tests failed\n");
    }

	return okay ? 0 : -1;
}
