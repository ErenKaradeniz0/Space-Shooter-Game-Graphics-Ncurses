#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>

#define BORDER_SIZE 10
#define MAX_X 1024
#define MAX_Y 768
#define SIDE_BAR_WIDTH 192 // 24 * 8

#define ROCKET_WIDTH 32 // 6 * 8
#define ROCKET_HEIGHT 80

#define SPACE_SHIP_WIDTH 80  // 6 * 8
#define SPACE_SHIP_HEIGHT 64 // 8 * 8

#define BULLET_SPEED 8
#define MAX_BULLETS 30

#define ROCKET_SPEED 8
#define MAX_ROCKETS 6
#define ROCKET_MOVE_DELAY 20
#define BULLET_MOVE_DELAY 2

#define RAND_MAX 112 // 944 / 8 test
typedef struct
{
    int x;
    int y;
    int active;  // Flag to bullet is active or not
    int avaible; // Flag to bullet is avaible to shot
} Bullet;

typedef struct
{
    int x;
    int y;
    int active; // Flag to rocket is active or not
} Rocket;

Bullet bullets[MAX_BULLETS];
Rocket rockets[MAX_ROCKETS];

int rocketMoveCounter = 0; // Counter to control rocket movement speed
int bulletMoveCounter = 0; // Counter to control bullet movement speed

int quit_flag = 0;
int pause_flag = 0;
char current_key = '1';
int bullet_count = MAX_BULLETS;
int ship_x;
int ship_y;
int score = 0;
char score_str[3];   // maximum number of digits is 3
char bullets_str[2]; // maximum number of digits is 2
TTF_Font *font = NULL;

void clear_screen(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black
    SDL_RenderClear(renderer);
}

void SDL_Clear_Rect(SDL_Renderer *renderer, int x, int y, int x1, int x2)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color
    SDL_Rect clearRect = {x, y, x1, x2};
    SDL_RenderFillRect(renderer, &clearRect);
}

void drawBoundaries(SDL_Renderer *renderer)
{
    SDL_Color textColor = {255, 255, 255, 255}; // White color
    // Draw top border
    SDL_Rect topBorder = {0, 0, MAX_X, BORDER_SIZE};
    SDL_RenderFillRect(renderer, &topBorder);

    // Draw bottom border
    SDL_Rect bottomBorder = {0, MAX_Y - BORDER_SIZE, MAX_X, BORDER_SIZE};
    SDL_RenderFillRect(renderer, &bottomBorder);

    // Draw left border
    SDL_Rect leftBorder = {0, BORDER_SIZE, BORDER_SIZE, MAX_Y - 2 * BORDER_SIZE};
    SDL_RenderFillRect(renderer, &leftBorder);

    // Draw SideBar
    SDL_Rect SideBar = {SIDE_BAR_WIDTH, BORDER_SIZE, BORDER_SIZE, MAX_Y - 2 * BORDER_SIZE};
    SDL_RenderFillRect(renderer, &SideBar);

    // Draw right border
    SDL_Rect rightBorder = {MAX_X - BORDER_SIZE, BORDER_SIZE, BORDER_SIZE, MAX_Y - 2 * BORDER_SIZE};
    SDL_RenderFillRect(renderer, &rightBorder);
}

// Function to convert an integer to its string representation
int int_to_string(int num, char *buffer)
{
    int i = 0;
    int digits = 0; // Variable to store the number of digits

    if (num == 0)
    {
        buffer[i++] = '0';
        digits = 1; // If the number is zero, it has one digit
    }
    else
    {
        // Calculate the number of digits
        int temp = num;
        while (temp != 0)
        {
            digits++;
            temp /= 10;
        }

        // Convert each digit to character and store in the buffer
        while (num != 0)
        {
            int digit = num % 10;
            buffer[i++] = '0' + digit;
            num /= 10;
        }
    }
    buffer[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    return digits; // Return the number of digits
}

void graphics_write_string(SDL_Renderer *renderer, int x, int y, const char *text, TTF_Font *font)
{
    SDL_Color textColor = {255, 255, 255}; // White color
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
    if (surface == NULL)
    {
        printf("Failed to render text! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL)
    {
        printf("Failed to create texture! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dstRect = {x, y, surface->w, surface->h}; // Position and size
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void printScore(SDL_Renderer *renderer, int x, int y)
{
    SDL_Clear_Rect(renderer, x, y, 18, 16);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
    int num_digits = int_to_string(score, score_str);
    graphics_write_string(renderer, x, y, score_str, font);
}

void bullet_counter()
{
    bullet_count = 0;
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].avaible)
        {
            bullet_count += 1;
        }
    }
}

void printBulletCount(SDL_Renderer *renderer, int x, int y)
{

    SDL_Clear_Rect(renderer, x, y, 18, 16);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
    int num_digits = int_to_string(bullet_count, bullets_str);
    graphics_write_string(renderer, x, y, bullets_str, font);
    if (bullet_count < 10)
        graphics_write_string(renderer, x + 8, y, " ", font);
}

void info(SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set color to draw (white)
    graphics_write_string(renderer, 16, 15, "Welcome!", font);
    graphics_write_string(renderer, 16, 30, "Save the World!", font);
    graphics_write_string(renderer, 16, 45, "by Eren Karadeniz", font);
    graphics_write_string(renderer, 16, 60, "200101070", font);

    graphics_write_string(renderer, 16, 85, "Keys", font);
    graphics_write_string(renderer, 16, 100, "A to move left", font);
    graphics_write_string(renderer, 16, 115, "D to move right", font);
    graphics_write_string(renderer, 16, 130, "Space to Shot", font);
    graphics_write_string(renderer, 16, 145, "Q to quit game", font);
    graphics_write_string(renderer, 16, 160, "R to restart game", font);
    graphics_write_string(renderer, 16, 175, "P to pause game", font);
    graphics_write_string(renderer, 16, 190, "Win after reach", font);
    graphics_write_string(renderer, 16, 205, "25 Score", font);
}

void intro(SDL_Renderer *renderer)
{

    drawBoundaries(renderer);

    info(renderer, font);

    graphics_write_string(renderer, 16, 250, "Bullets:", font);
    printBulletCount(renderer, 88, 250);

    graphics_write_string(renderer, 16, 265, "Score:", font);
    printScore(renderer, 80, 265);
}

// Draw A
void draw_a(SDL_Renderer *renderer, int x, int y, int w, int h)
{
    SDL_RenderDrawLine(renderer, x, y, x + w + 3, y + h + 3);
    SDL_RenderDrawLine(renderer, x, y, x - w - 3, y + h + 3);

    SDL_RenderDrawLine(renderer, x - 10, y + 10, x - 10, y + 10 + h + 5);
    SDL_RenderDrawLine(renderer, x + 10, y + 10, x + 10, y + 10 + h + 5);
    SDL_RenderDrawLine(renderer, x - 10, y + 15, x + 10, y + 15);
}

void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set color to white
    for (int x = -radius; x <= radius; x++)
    {
        int y = (int)sqrt(radius * radius - x * x);              // Calculate y-coordinate based on the equation of a circle
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y); // Draw top half of circle
        SDL_RenderDrawPoint(renderer, centerX + x, centerY - y); // Draw bottom half of circle
    }
}

void drawSpaceship(SDL_Renderer *renderer, int x, int y, int w, int h)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color
    // Draw A
    draw_a(renderer, x, y, w, h);
    draw_a(renderer, x + 70, y, w, h);
    draw_a(renderer, x, y + 20, w, h);
    draw_a(renderer, x + 70, y + 20, w, h);

    // Draw I
    SDL_RenderDrawLine(renderer, x + 35, y, x + 35, y + h + 6);
    // Draw -
    SDL_RenderDrawLine(renderer, x + 35 - w + 1, y, x + 35 + w - 1, y);
    SDL_RenderDrawLine(renderer, x + 35 - w + 1, y + h + 6, x + 35 + w - 1, y + h + 6);

    // Draw /
    SDL_RenderDrawLine(renderer, x + 30, y + 20, x + 30 - w - 3, y + 20 + h + 3);
    // Draw '\'
    SDL_RenderDrawLine(renderer, x + 40, y + 20, x + 40 + w + 3, y + 20 + h + 3);
    // Draw -
    SDL_RenderDrawLine(renderer, x + 30, y + 22, x + 30 + w + 6, y + 22);

    // Draw /
    SDL_RenderDrawLine(renderer, x + 10, y + 45, x + 10 - w, y + 45 + h);
    // Draw o

    drawCircle(renderer, x + 20, y + 45 - w - 2, 5);
    drawCircle(renderer, x + 35, y + 45 - w - 2, 5);
    drawCircle(renderer, x + 50, y + 45 - w - 2, 5);

    // Draw '\'
    SDL_RenderDrawLine(renderer, x + 60, y + 45, x + 60 + w, y + 45 + h);
}

void clearSpaceship(SDL_Renderer *renderer, int x, int y, int w, int h)
{
    // Calculate bottom-right corner coordinates
    int x2 = x + 92;
    int y2 = y + 54; // Maximum y-coordinate for the spaceship

    // Set the color to clear (usually you would set this to your background color)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color

    // Define the rectangle to clear
    SDL_Rect clearRect = {x - 10, y, x2 - x, y2 - y};

    // Clear the rectangle covering the spaceship
    SDL_RenderFillRect(renderer, &clearRect);
}

void drawCharacter(SDL_Renderer *renderer, int x, int y)
{
    // Draw the '^' character
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set color to draw (white)

    // Draw the points to form the '^' character
    SDL_RenderDrawPoint(renderer, x, y);         // Top center point
    SDL_RenderDrawPoint(renderer, x - 1, y + 1); // Upper left point
    SDL_RenderDrawPoint(renderer, x - 2, y + 2); // Upper left point
    SDL_RenderDrawPoint(renderer, x - 3, y + 3); // Upper left point
    SDL_RenderDrawPoint(renderer, x + 1, y + 1); // Upper right point
    SDL_RenderDrawPoint(renderer, x + 2, y + 2); // Upper right point
    SDL_RenderDrawPoint(renderer, x + 3, y + 3); // Upper right point
}

void moveBullet(SDL_Renderer *renderer, int index)
{
    if (bulletMoveCounter % BULLET_MOVE_DELAY == 0)
    {

        // Clear previous bullet position
        SDL_Clear_Rect(renderer, bullets[index].x - 3, bullets[index].y, 7, 4);
        if (bullets[index].y > 20)
        {

            bullets[index].y -= BULLET_SPEED; // Move the bullet upwards
            // Draw '^' character at the new position
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);        // Set color to draw (white)
            drawCharacter(renderer, bullets[index].x, bullets[index].y); // Draw the bullet
        }
        else
            bullets[index].active = 0;
    }
}

void move_bullets(SDL_Renderer *renderer)
{
    // Move all active bullets
    for (int index = 0; index < MAX_BULLETS; index++)
    {
        if (!pause_flag)
        {
            if (bullets[index].active && !bullets[index].avaible)
            {
                drawCharacter(renderer, bullets[index].x, bullets[index].y); // Draw the bullet
                moveBullet(renderer, index);
            }
        }
    }

    // Increment the bullet move counter
    bulletMoveCounter++;

    // Reset the counter to prevent overflow
    if (bulletMoveCounter >= BULLET_MOVE_DELAY)
        bulletMoveCounter = 0;
}

void shot_bullet(Bullet *bullet)
{
    bullet->active = 1;
    bullet->avaible = 0;
    bullet->x = ship_x + 32; // Adjust bullet position to appear from spaceship's center
    bullet->y = ship_y - 16;
}

void drawRocket(SDL_Renderer *renderer, int x, int y)
{
    // Draw \||/
    SDL_RenderDrawLine(renderer, x, y - 10 - 8, x + 10, y - 10);
    SDL_RenderDrawLine(renderer, x + 10, y - 8, x + 10, y - 28);
    SDL_RenderDrawLine(renderer, x + 20, y - 8, x + 20, y - 28);
    SDL_RenderDrawLine(renderer, x + 20, y - 8, x + 30, y - 18);

    // Draw ___
    SDL_RenderDrawLine(renderer, x, y, x + 30, y);

    // Draw |o|
    SDL_RenderDrawLine(renderer, x, y + 25, x, y - 20);
    SDL_Rect circle = {x + 15 - 4, y + 10 - 4, 8, 8}; // Rectangle for circle
    SDL_RenderDrawRect(renderer, &circle);
    SDL_RenderDrawLine(renderer, x + 30, y + 25, x + 30, y - 20);

    // Draw '\'
    SDL_RenderDrawLine(renderer, x, y + 15, x + 15, y + 30);
    // Draw /
    SDL_RenderDrawLine(renderer, x + 30, y + 15, x + 15, y + 30);
}

void clearRocket(SDL_Renderer *renderer, int x, int y)
{
    // Calculate bottom-right corner coordinates
    int x2 = x + ROCKET_WIDTH;
    int y2 = y + ROCKET_HEIGHT; // Maximum y-coordinate for the spaceship

    // Set the color to clear (usually you would set this to your background color)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color

    // Define the rectangle to clear
    SDL_Rect clearRect = {x, y - 40, x2 - x, y2 - y};

    // Clear the rectangle covering the spaceship
    SDL_RenderFillRect(renderer, &clearRect);
}

unsigned int get_system_timer_value()
{
    unsigned int val;
    // Read the value of the system timer (assuming x86 architecture)
    asm volatile("rdtsc" : "=a"(val));
    return val;
}

// Define some global variables for the random number generator
static unsigned long next;

// A function to generate a pseudo-random integer
int rand(void)
{
    next = get_system_timer_value();
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % RAND_MAX;
}

int randRocketAxis()
{
    int min_x = SIDE_BAR_WIDTH / 8 + 1;
    int max_x = MAX_X / 8 - ROCKET_WIDTH / 8;
    int x = rand(); // 1 - 88
    while (min_x > x || x > max_x)
    {
        x = rand();
    }
    return x + 8;
}

// Function to generate a single rocket from passive rocket
void generateRocket(Rocket *rocket)
{
    int newRocketX, newRocketY;
    int collisionDetected;

    do
    {
        // Generate random position for the new rocket
        newRocketX = 8 * randRocketAxis(); // Adjust range to prevent overflow
        newRocketY = 52;                   // Adjust range as needed

        // Check for collision with existing rockets based on X position only
        collisionDetected = 0;
        for (int j = 0; j < MAX_ROCKETS; j++)
        {
            if (rockets[j].active &&
                (newRocketX >= rockets[j].x - 20 - ROCKET_WIDTH && newRocketX <= rockets[j].x + ROCKET_WIDTH)) // Check only X position
            {
                collisionDetected = 1;
                break;
            }
        }
    } while (collisionDetected);

    // Set the position of the new rocket
    rocket->x = newRocketX;
    rocket->y = newRocketY;
    rocket->active = 1;
}

void generate_rockets()
{
    // Generate new rockets if there are inactive rockets
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (!rockets[i].active)
        {
            generateRocket(&rockets[i]);
        }
    }
}

void moveRocket(SDL_Renderer *renderer, int index)
{
    if (rocketMoveCounter % ROCKET_MOVE_DELAY == 0)
    {                                                              // Move the rocket every ROCKET_MOVE_DELAY frames
        clearRocket(renderer, rockets[index].x, rockets[index].y); // Clear previous rocket position
        rockets[index].y += ROCKET_SPEED;                          // Move the rocket downwards
        drawRocket(renderer, rockets[index].x, rockets[index].y);
    }
}

void move_rockets(SDL_Renderer *renderer)
{
    // Draw and move the rocket
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (!pause_flag)
        {
            if (rockets[i].active)
            {
                drawRocket(renderer, rockets[i].x, rockets[i].y);
                moveRocket(renderer, i);
            }
        }
    }

    // Increment the rocket move counter
    rocketMoveCounter++;
    // Reset the counter to prevent overflow
    if (rocketMoveCounter >= ROCKET_MOVE_DELAY)
        rocketMoveCounter = 0;
    if (current_key != 'p')
    {
        generate_rockets();
    }
}

void initBullets()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].x = 1;
        bullets[i].y = 1;
        bullets[i].active = 0;
        bullets[i].avaible = 1;
    }
}

void initRockets()
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        int newRocketX, newRocketY;
        int collisionDetected;

        do
        {
            // Generate random position for the new rocket
            newRocketX = 8 * randRocketAxis();
            newRocketY = 52;

            // Check for collision with existing rockets based on X position only
            collisionDetected = 0;
            for (int j = 0; j < i; j++)
            {
                if (rockets[j].active &&
                    (newRocketX >= rockets[j].x - ROCKET_WIDTH && newRocketX <= rockets[j].x + ROCKET_WIDTH)) // Check only X position
                {
                    collisionDetected = 1;
                    i = 0;
                    break;
                }
            }
        } while (collisionDetected);

        // Set the position of the new rocket
        rockets[i].x = newRocketX;
        rockets[i].y = newRocketY;
        rockets[i].active = 1;
    }
}

int collisionBullet(SDL_Renderer *renderer)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            for (int j = 0; j < MAX_ROCKETS; j++)
            {
                if (rockets[j].active &&
                    bullets[i].x >= rockets[j].x - 8 && bullets[i].x < rockets[j].x + ROCKET_WIDTH &&
                    bullets[i].y >= rockets[j].y && bullets[i].y < rockets[j].y + ROCKET_HEIGHT)
                {
                    score += 1;

                    printScore(renderer, 80, 265);
                    bullets[i].active = 0; // Deactivate bullet
                    rockets[j].active = 0; // Deactivate rocket
                    SDL_Clear_Rect(renderer, bullets[i].x - 3, bullets[i].y, 7, 4);
                    clearRocket(renderer, rockets[j].x, rockets[j].y);
                    break;
                }
            }
        }
    }
}

void gameOver(SDL_Renderer *renderer)
{
    clear_screen(renderer);
    info(renderer, font);
    drawBoundaries(renderer);
    graphics_write_string(renderer, (MAX_X - SIDE_BAR_WIDTH) / 2 + 30, MAX_Y / 2 + 15, "You lost, Press R for Play Again", font);
    graphics_write_string(renderer, (MAX_X - SIDE_BAR_WIDTH) / 2 + 140, MAX_Y / 2 + 30, "Score: ", font);
    graphics_write_string(renderer, (MAX_X - SIDE_BAR_WIDTH) / 2 + 200, MAX_Y / 2 + 30, score_str, font);
    SDL_RenderPresent(renderer);
}

// Function to check for collision between rocket and spaceship
void collisionSpaceShip(SDL_Renderer *renderer)
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        // Check if any of the edges of the rocket box lie outside the spaceship box

        if (ship_x <= rockets[i].x + ROCKET_WIDTH - 1 && ship_x + SPACE_SHIP_WIDTH - 1 >= rockets[i].x && rockets[i].y + ROCKET_HEIGHT - 40 >= ship_y)
        {
            quit_flag = 1;
            gameOver(renderer);
            graphics_write_string(renderer, (MAX_X - SIDE_BAR_WIDTH) / 2 + 30, MAX_Y / 2, "Spaceship destroyed by rocket", font);
        }
    }
}

void init(SDL_Renderer *renderer)
{
    // Set background color

    clear_screen(renderer);
    initBullets();
    initRockets();
    intro(renderer);
    drawBoundaries(renderer);

    ship_x = (MAX_X + SIDE_BAR_WIDTH) / 2 - SPACE_SHIP_WIDTH / 4; // base x of spaceship 49th pixel
    ship_y = MAX_Y - SPACE_SHIP_HEIGHT;                           // base y of spaceship 87th pixel
    SDL_RenderPresent(renderer);
}

void quitGame(SDL_Renderer *renderer)
{
    clear_screen(renderer);
    drawBoundaries(renderer);
    info(renderer, font);
    graphics_write_string(renderer, MAX_X / 2, MAX_Y / 2, "Press R for Play Again", font);
}

void restartGame(SDL_Renderer *renderer)
{
    init(renderer); // Initialize the game
}

void handleUserInput(SDL_Renderer *renderer, char current_key, Bullet bullets[MAX_BULLETS])
{
    if (!pause_flag)
    {
        switch (current_key)
        {
        case 'a':
            if (ship_x - 1 > SIDE_BAR_WIDTH + 20)
            {
                clearSpaceship(renderer, ship_x, ship_y, 4, 4);
                ship_x -= 8;
            }
            break;
        case 'd':
            if (ship_x + SPACE_SHIP_WIDTH < MAX_X - 16)
            {
                clearSpaceship(renderer, ship_x, ship_y, 4, 4);
                ship_x += 8;
            }
            break;
        case ' ':
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (!bullets[i].active && bullets[i].avaible)
                {
                    shot_bullet(&bullets[i]);
                    bullet_counter();
                    printBulletCount(renderer, 88, 250);
                    break;
                }
            }
            break;
        case 'q':
            SDL_Quit();
            break;
        case 'r':
            score = 0;
            quit_flag = 0;
            bullet_count = MAX_BULLETS;
            restartGame(renderer); // Restart the game
            break;
        case 'p':
            pause_flag = !pause_flag; // Toggle pause_flag
            if (pause_flag)
            {
                graphics_write_string(renderer, MAX_X / 2, MAX_Y / 2, "Paused, Press p to continue", font);
            }
            break;
        }
    }
    else
    {
        if (current_key == 'p')
        {
            pause_flag = 0;
            SDL_Clear_Rect(renderer, MAX_X / 2, MAX_Y / 2, 245, 20);
        }
    }
}

void winGame(SDL_Renderer *renderer)
{
    clear_screen(renderer);
    info(renderer, font);
    drawBoundaries(renderer);
    graphics_write_string(renderer, MAX_X / 2, MAX_Y / 2, "You Win!", font);
    graphics_write_string(renderer, MAX_X / 2, MAX_Y / 2 + 15, "Press R for Play Again", font);
    // Update screen
    SDL_RenderPresent(renderer);
}

int continueGame(SDL_Renderer *renderer)
{
    // Check if all rockets have reached the bottom of the screen

    int rocketReachedBottom = 0;
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (rockets[i].y + 45 >= MAX_Y)
        {
            rocketReachedBottom = 1;
            if (rocketReachedBottom)
            {
                quit_flag = 1;
                gameOver(renderer);
                graphics_write_string(renderer, (MAX_X - SIDE_BAR_WIDTH) / 2 + 30, MAX_Y / 2, "Rockets Reached Bottom.", font);
                return 0;
            }
        }
    }

    if (score == 25)
    {
        quit_flag = 1;
        winGame(renderer);
        score = 0;
        return 0;
    }

    return 1;
}

void busy_wait(unsigned int milliseconds)
{
    // Calculate the number of iterations needed for the desired milliseconds
    unsigned int iterations = milliseconds * 10000; // Adjust this value as needed based on your system's clock speed

    // Execute an empty loop for the specified number of iterations
    for (unsigned int i = 0; i < iterations; ++i)
    {
        // Do nothing, just wait
    }
}
char keyboard_read()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_KEYDOWN)
    {
        // Get the current key pressed
        char currentKey = event.key.keysym.sym;
        return currentKey;
    }
}
int main()
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    window = SDL_CreateWindow("Print Character", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, MAX_X, MAX_Y, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    // Load font
    font = TTF_OpenFont("arial.ttf", 18);
    if (font == NULL)
    {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return 1;
    }

    init(renderer);

    // Main event loop

    // //Game loop
    while (1)
    {
        while (quit_flag == 0 && continueGame(renderer))
        {

            current_key = keyboard_read();

            handleUserInput(renderer, current_key, bullets);

            drawSpaceship(renderer, ship_x, ship_y, 4, 4);

            move_bullets(renderer);
            move_rockets(renderer);
            //  Check for collision between bullets and rockets
            collisionBullet(renderer);
            collisionSpaceShip(renderer);

            // Update screen
            SDL_RenderPresent(renderer);
            busy_wait(800); // Wait for 50 milliseconds using busy wait
        }

        current_key = keyboard_read();
        if (current_key == 'r')
        {
            quit_flag = 0;
            bullet_count = MAX_BULLETS;
            restartGame(renderer); // Restart the game
        }
        if (current_key == 'q')
        {
            SDL_Quit();
        }
        busy_wait(50);
    }
    return 0;
}
