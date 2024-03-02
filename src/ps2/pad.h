
void initPads();
void readInput();

// 1 if the start button has been pressed.
extern int startButtonPressed;

extern int acceptPadButton;
extern int backPadButton;

struct PadBgdaData
{
    int failedReadCountdown;
    int RLUD_buttons;
    int updatedButtons;
    int updatedButtons2;
    float r_stick_x;
    float r_stick_y;
    float l_stick_x;
    float l_stick_y;
    int pad_0x20;
    float pad_right;
    float pad_left;
    float pad_up;
    float pad_down;
    float pad_l1;
    float pad_l2;
    float pad_circle;
    float pad_square;
    float pad_triangle;
    float pad_cross;
    float pad_r1;
    float pad_r2;
};

extern PadBgdaData convertedControllerInput[2];

#define DA_PAD_RIGHT 0x2000
#define DA_PAD_LEFT 0x8000
#define DA_PAD_UP 0x1000
#define DA_PAD_DOWN 0x4000

#define DA_PAD_CIRCLE 0x0020
#define DA_PAD_SQUARE 0x0080
#define DA_PAD_TRIANGLE 0x0010
#define DA_PAD_CROSS 0x0040

#define DA_PAD_R2 0x0002
#define DA_PAD_L1 0x0004
#define DA_PAD_R1 0x0008
#define DA_PAD_L2 0x0001

#define DA_RSTICK_UP    0x100000
#define DA_RSTICK_DOWN  0x200000
#define DA_RSTICK_LEFT  0x400000
#define DA_RSTICK_RIGHT 0x800000
