#include "xo.h"

char winner;
char locations = 9;
char posNum = 0;
struct move bestMove;
char turn = 0;

int main(void){
	
	PortF_Init();		//	Call initialization of port PF4, PF3, PF2, PF1, PF0    
  TExaS_Init(SSI0_Real_Nokia5110_Scope);		//	Set system clock to 80 MHz
	
  Nokia5110_Init();		//	Initialization of NOKIA 5110
  Nokia5110_ClearBuffer();
	Nokia5110_DisplayBuffer();  
	
	initializeXOGrid(); 	//	Initialize XO grid by clearing its positions
	Nokia5110_SetCursor(0, 1);
	Nokia5110_OutString("1 VS 1 : SW1");
	Nokia5110_SetCursor(1, 4);
	Nokia5110_OutString("VS AI : SW2");

  while(1){
		if (!(GPIO_PORTF_DATA_R&0x10))	// sw1
		{
			Nokia5110_Clear();
			Nokia5110_PrintBMP(21, 44, xoGrid, 0);		// Printing XO Grid
			Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, dash, 0);		// Initialize the cursor to the first position
			Nokia5110_DisplayBuffer();     // Draw buffer
			while(!(GPIO_PORTF_DATA_R&0x10));
			twoVStwo();
		}
		if (!(GPIO_PORTF_DATA_R&0x01)) // sw2
		{
			Nokia5110_Clear();
			Nokia5110_PrintBMP(21, 44, xoGrid, 0);		// Printing XO Grid
			Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, dash, 0);		// Initialize the cursor to the first position
			Nokia5110_DisplayBuffer();     // Draw buffer
			while(!(GPIO_PORTF_DATA_R&0x01));
			vsAI();
		}
		
  }
}
void twoVStwo(void)
{
	while (1) {
		if (!(GPIO_PORTF_DATA_R&0x10)) {	// sw1
			moveToNextPos();
			Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, dash, 0);
			Nokia5110_DisplayBuffer();
			while(!(GPIO_PORTF_DATA_R&0x10));
		}
		if (!(GPIO_PORTF_DATA_R&0x01)) {		// sw2
			if (isNotFull() == 0)		//	Check if the grid is full or not before making the move
			{
				announceResult(winner);
			}
			if (turn % 2 == 0) {
				Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, xPlay, 0);
				posArr[posNum].own = 1;
				turn ++;
				locations --;
				moveToNextPos();
				Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, dash, 0);
			}
			else {
				Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, oPlay, 0);
				posArr[posNum].own = -1;
				turn ++;
				locations --;
				moveToNextPos();
				Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, dash, 0);
			}
			Nokia5110_DisplayBuffer();
			winner = checkForWinner();
			if ((winner == 10) || (winner == -10)) {
				announceResult(winner);
			}
			while(!(GPIO_PORTF_DATA_R&0x01));
		}
	}
}
void vsAI(void)
{
	while (1){
		if (!(GPIO_PORTF_DATA_R&0x10))		// If switch 1 from port F is pressed, move the cursor to the next available position
		{
			moveToNextPos();
			Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, dash, 0);		// Printing the cursor
			Nokia5110_DisplayBuffer();
			while(!(GPIO_PORTF_DATA_R&0x10));
		}

		if (!(GPIO_PORTF_DATA_R&0x01))		// If switch 2 from port F is pressed, place X in the selected position and let the AI make its move
		{
			Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, xPlay, 0);		// Printing X
			posArr[posNum].own = 1;
			locations --;
			Nokia5110_DisplayBuffer();		// Draw buffer

			winner = checkForWinner();		// Check if X player is won, if so announce the winner
			if (winner == 10)
			{
				announceResult(winner);
			}

			aiTurn();		// AI makes its move
			winner = checkForWinner();		// Check if O player (AI) is won, if so announce the winner
			if (winner == -10)
			{
				announceResult(winner);
			}

			while(!(GPIO_PORTF_DATA_R&0x01));
		}
	}
}

void Delay100ms(unsigned long count){
	unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}

void PortF_Init(void){
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}

void initializeXOGrid(void) {
	int j;
	for (j = 0; j < 10; j++)
	{
		posArr[j].own = 0;
	}
}

char checkForWinner(void) {
	int i = 0;
	//	Diagonal check top left to bottom right
	if ((posArr[0].own == posArr[4].own) && (posArr[4].own == posArr[8].own))
	{
		if (posArr[0].own == 1)
			return 10;
		if (posArr[0].own == -1)
			return -10;
	}
	//	Diagonal check top right to bottom left
	else if ((posArr[2].own == posArr[4].own) && (posArr[4].own == posArr[6].own))
	{
		if (posArr[2].own == 1)
			return 10;
		if (posArr[2].own == -1)
			return -10;
	}
	// Horizontal check
	for (i = 0; i < 7; i+=3)
	{
		if ((posArr[i].own == posArr[i + 1].own) && (posArr[i + 1].own == posArr[i + 2].own))
		{
			if (posArr[i].own == 1)
				return 10;
			if (posArr[i].own == -1)
				return -10;
		}
	}
	// Vertical check
	for (i = 0; i < 3; i++)
	{
		if ((posArr[i].own == posArr[i + 3].own) && (posArr[i + 3].own == posArr[i + 6].own))
		{
			if (posArr[i].own == 1)
				return 10;
			if (posArr[i].own == -1)
				return -10;
		}
	}
	return 0;
}

char isNotFull(void){
	if (locations == 0)
		return 0;
	else
		return 1;
}

void moveToNextPos() {
	int i = 0;

	if (posArr[posNum].own == 0)
	{
		Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, noDash, 0);
	}
	for (i = 0; i < 9; i++)
	{
		posNum++;
		if (posNum == 9)
		{
			posNum = 0;
		}
		if (posArr[posNum].own == 0)
		{
			return;
		}
	}
}

void aiTurn(void)
{
	char i;
	char j;
	char score;
	char bestScore = 127;
	if (isNotFull() == 0)		//	Check if the grid is full or not before making the move
	{
		announceResult(winner);
	}
	for (i = 0; i < 9; i++)
	{
		if (posArr[i].own == 0)
		{
			posArr[i].own = -1;
			score = minimax(posArr, 0, 1);
			posArr[i].own = 0;
			if (score < bestScore)
			{
				bestScore = score;
				bestMove.xPos = posArr[i].xPos;
				bestMove.yPos = posArr[i].yPos;
				j = i;
			}
		}
	}
	Nokia5110_PrintBMP(bestMove.xPos, bestMove.yPos, oPlay, 0);		//	After choosing the best move, print O in the selected postion
	Nokia5110_DisplayBuffer();
	posArr[j].own = -1;
	locations --;
	moveToNextPos();		//	Place the cursor in the next empty position
	Nokia5110_PrintBMP(posArr[posNum].xPos, posArr[posNum].yPos, dash, 0);
	Nokia5110_DisplayBuffer();
}

char minimax(struct position *board,char depth, char maxOrMin)
{
	int score;
	int moveScore;
	char i;
	char j;
	char bestMinScore = 127;
	char bestMaxScore = -127;
	score = checkForWinner();
	
	if (score == 10){
		return score;
	}
	if (score == -10){
		return score;
	}
	if (isNotFull() == 0){
		return 0;
	}
	
	if (maxOrMin == 0)	//	For the minimizing player (X- player)
	{
		for (i = 0; i < 9; i++)
		{
			if (posArr[i].own == 0)
			{
				posArr[i].own = -1;
				moveScore = minimax(posArr, depth+1, 1);
				posArr[i].own = 0;
				if (moveScore < bestMinScore)
				{
					bestMinScore = moveScore;
				}
			}
		}
		return bestMinScore + depth;
	}
	else		//	For the maximizing player (O- player) AI
	{
		for (j = 0; j < 9; j++)
		{
			if (posArr[j].own == 0)
			{
				posArr[j].own = 1;
				moveScore = minimax(posArr, depth + 1, 0);
				posArr[j].own = 0;
				if (moveScore > bestMaxScore)
				{
					bestMaxScore = moveScore - depth;
				}
				
			}
		}
		return bestMaxScore;
	}
}
void announceResult(char winner)
{
		Delay100ms(3);		// delay 3 sec at 50 MHz
		Nokia5110_Clear();		// clear the screen to anounce the result
		Nokia5110_SetCursor(1, 0);
		Nokia5110_OutString("GAME OVER");
		Nokia5110_SetCursor(1, 1);
		if (winner == 0) {
			RED_On();
			Nokia5110_OutString("It is a TIE");
		}
		else if (winner == 10) {
			BLUE_On();
			Nokia5110_SetCursor(3, 1);
			Nokia5110_OutString("X WON");
		}
		else if (winner == -10) {
			GREEN_On();
			Nokia5110_SetCursor(3, 1);
			Nokia5110_OutString("O WON");
		}
		Nokia5110_SetCursor(0, 3);
		Nokia5110_OutString("FOR NEW GAME");
		Nokia5110_SetCursor(1, 4);
		Nokia5110_OutString("PRESS RESET");
		while(1);
}

void BLUE_On(void){
  GPIO_PORTF_DATA_R |= 0x04;
}

void GREEN_On(void){
  GPIO_PORTF_DATA_R |= 0x08;
}

void RED_On(void){
  GPIO_PORTF_DATA_R |= 0x02;
}
