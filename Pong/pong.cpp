/*

Author: Yolanda Haynes
Cite: LazyFoo Productions SDL tutorials <http://lazyfoo.net/tutorials/SDL/>

*/

//Using SDL, SDL_image, SDL_ttf...etc
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <time.h>
using namespace std;

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

Uint32 start_time = 0, end_time = 0;
int delta;

int player1_score;
int player2_score;
bool p1_scored;
bool p2_scored;

//Texture wrapper class
class LTexture
{
    public:
        //Initializes variables
        LTexture();

        //Deallocates memory
        ~LTexture();

        //Loads image at specified path
        bool loadFromFile( string path );

        //Creates image from font string
        bool loadFromRenderedText( string textureText, SDL_Color textColor );

        //Deallocates texture
        void free();

        //Set color modulation
        void setColor( Uint8 red, Uint8 green, Uint8 blue );

        //Renders texture at given point
        void render( int x, int y, SDL_Rect* clip = NULL);

        //Gets image dimensions
        int getWidth();
        int getHeight();

    private:
        //The actual hardware texture
        SDL_Texture* mTexture;

        //Image dimensions
        int mWidth;
        int mHeight;
};

//The paddle that will up and down
class Paddle
{
    private:
        //The velocity of the paddles
        int mVelX_P1, mVelY_P1;
        int mVelX_P2, mVelY_P2;

    public:
        //The dimensions of the paddle
        static const int PADDLE_WIDTH = 10;
        static const int PADDLE_HEIGHT = 110;

        //Maximum axis velocity of the paddle
        static const int PADDLE_VEL = 10;

        //Paddle collision box
        SDL_Rect pad_P1;
        SDL_Rect pad_P2;

        //Initializes the variables
        Paddle();

        //Takes key presses and adjusts the paddle's velocity
        void handleEvent( SDL_Event& e );

        //Moves the paddle
        void move();

        //Shows the paddles
        void render1();
        void render2();
};

class Ball
{
    public:
        //The dimensions of the ball
        static const int BALL_WIDTH = 20;
        static const int BALL_HEIGHT = 20;
        static const int BALL_SPEED = 10;

        //The velocity of the ball
        int BallXVel;
        int BallYVel;

        //Ball collision box
        SDL_Rect cBall;

        //Initializes the variables of the ball
        Ball();

        //Takes key presses and adjusts the paddle's velocity
        void startEvent( SDL_Event& e );

        //Move ball
        void moveBall();

        //Shows ball
        void render();

        //reset ball
        void reset();

};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Box collision detector
bool checkCollision( SDL_Rect a, SDL_Rect b );

//ball angle
int Ball_angle(int p_y, int b_y);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Globally used font
TTF_Font *gFont = NULL;

//Scene sprites
SDL_Rect gP1_Paddle;
SDL_Rect gP2_Paddle;
SDL_Rect gBall;

//LTexture Paddle/Ball Texture;
LTexture gPaddleTexture;
LTexture gBallTexture;
LTexture gBackgroundTexture;

//The sound effects that will be used
Mix_Chunk *gMiss = NULL;
Mix_Chunk *gWall = NULL;
Mix_Chunk *gPaddle = NULL;

//Rendered texture
LTexture gTextTexture;

LTexture::LTexture()
{
    //Initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture()
{
    //Deallocate
    free();
}

bool LTexture::loadFromFile( std::string path )
{
    //Get rid of preexisting texture
    free();

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Color key image
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0xFF, 0xE3, 0xA0 ) );

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    //Return success
    mTexture = newTexture;
    return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( string textureText, SDL_Color textColor )
{
    //Get rid of preexisting texture
    free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }

    //Return success
    return mTexture != NULL;
}

void LTexture::free()
{
    //Free texture if it exists
    if( mTexture != NULL )
    {
        SDL_DestroyTexture( mTexture );
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
    //Modulate texture rgb
    SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::render( int x, int y, SDL_Rect* clip)
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };

    //Set clip rendering dimensions
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    //Render to screen
    SDL_RenderCopy ( gRenderer, mTexture, clip, &renderQuad);
}
int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}

Paddle::Paddle()
{
    pad_P1.x = 0;
    pad_P1.y = (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
    pad_P1.w = PADDLE_WIDTH;
    pad_P1.h = PADDLE_HEIGHT;

    pad_P2.x = SCREEN_WIDTH - PADDLE_WIDTH;
    pad_P2.y = (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
    pad_P2.w = PADDLE_WIDTH;
    pad_P2.h = PADDLE_HEIGHT;

    //Initialize the velocity
    mVelX_P1 = 0;
    mVelY_P1 = 0;
    mVelX_P2 = 0;
    mVelY_P2 = 0;
}
void Paddle::handleEvent( SDL_Event& e )
{
    //If a key was pressed
    if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_w: mVelY_P1 -= PADDLE_VEL; break;
            case SDLK_s: mVelY_P1 += PADDLE_VEL; break;
            case SDLK_UP: mVelY_P2 -= PADDLE_VEL; break;
            case SDLK_DOWN: mVelY_P2 += PADDLE_VEL; break;
        }
    }
    //If a key was released
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_w: mVelY_P1 += PADDLE_VEL; break;
            case SDLK_s: mVelY_P1 -= PADDLE_VEL; break;
            case SDLK_UP: mVelY_P2 += PADDLE_VEL; break;
            case SDLK_DOWN: mVelY_P2 -= PADDLE_VEL; break;
        }
    }
}

void Paddle::move()
{
    //Move the paddle up or down
    pad_P1.y += mVelY_P1;
    pad_P2.y += mVelY_P2;

    //If the paddle went too far up or down
    if( ( pad_P1.y < 0 ) || ( pad_P1.y + PADDLE_HEIGHT > SCREEN_HEIGHT ) )
    {
        //Move back
        pad_P1.y -= mVelY_P1;
    }

    if( ( pad_P2.y < 0 ) || ( pad_P2.y + PADDLE_HEIGHT > SCREEN_HEIGHT ) )
    {
        //Move back
        pad_P2.y -= mVelY_P2;
    }
}

void Paddle::render1()
{
    //Show the paddle
    gPaddleTexture.render( pad_P1.x, pad_P1.y, &gP1_Paddle );
}

void Paddle::render2()
{
    //Show the paddle
    gPaddleTexture.render( pad_P2.x, pad_P2.y, &gP2_Paddle );
}

Ball::Ball()
{
    //Initialize the offsets
    cBall.x = (SCREEN_WIDTH / 2) - (BALL_WIDTH / 2);
    cBall.y = (SCREEN_HEIGHT / 2) - (BALL_HEIGHT / 2);
    cBall.w = BALL_WIDTH;
    cBall.h = BALL_HEIGHT;

    //Initialize the velocity
    BallXVel = 0;
    BallYVel = 0;

}

void Ball::startEvent( SDL_Event& e)
{
    if ( e.type == SDL_KEYDOWN )
    {
        //Random velocity
        if( e.key.keysym.sym == SDLK_SPACE )
        {
            if (BallXVel == 0 && BallYVel == 0) //only if the ball is not moving already
            {
                if ( rand() % 2 == 0 )
                {
                    BallXVel += rand() % BALL_SPEED + 1;
                    BallYVel += rand() % BALL_SPEED; // * 2;
                }

                if ( rand() % 2 == 1 )
                {
                    BallXVel += rand () % (BALL_SPEED + 1) * -1;
                    BallYVel += (rand() % BALL_SPEED * 2 + 1) * -1;
                }

                //if (rand() % 2 == 0)
                //BallYVel = -1;
            }
        }
    }

}

void Ball::moveBall()
{
    //move the ball
    cBall.x += BallXVel;
    cBall.y += BallYVel;

    //the ball went to far up or down
    if( ( cBall.y < 0 ) || ( cBall.y + BALL_HEIGHT > SCREEN_HEIGHT ) )
    {
        //Move back
        BallYVel = -1 * BallYVel;
        Mix_PlayChannel( -1, gWall, 0 );
    }
}

void Ball::reset()
{
    //Initialize the offsets
    cBall.x = (SCREEN_WIDTH / 2) - (BALL_WIDTH / 2);
    cBall.y = (SCREEN_HEIGHT / 2) - (BALL_HEIGHT / 2);

    //Initialize the velocity
    BallXVel = 0;
    BallYVel = 0;

    //
    //player1_score
    //player2_score
}

void Ball::render()
{
    //Show ball
    gBallTexture.render( cBall.x , cBall.y , &gBall);
}

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }

        //Create window
        gWindow = SDL_CreateWindow( "Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create vsynced renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }

                 //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }

                //Initialize SDL_mixer
                if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }
            }
        }
    }

    return success;
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Open the font
    gFont = TTF_OpenFont( "Demun Lotion.ttf", 35 );
    if( gFont == NULL )
    {
        printf( "Failed to load Denum font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }

    //Load background texture
    if( !gBackgroundTexture.loadFromFile( "bg_1_1.png" ) )
    {
        printf( "Failed to load background texture image!\n" );
        success = false;
    }

    //Load sprite sheet texture
    if( !gPaddleTexture.loadFromFile( "sprites.png" ) )
    {
        printf( "Failed to load sprite sheet texture!\n" ); success = false;
    }
    else
    {
        //Set left sprite
        gP1_Paddle.x = 20;
        gP1_Paddle.y = 20;
        gP1_Paddle.w = 10;
        gP1_Paddle.h = 130;

        //Set right sprite
        gP2_Paddle.x = 70;
        gP2_Paddle.y = 20;
        gP2_Paddle.w = 10;
        gP2_Paddle.h = 130;
    }
    if( !gBallTexture.loadFromFile( "sprites.png" ) )
    {
        printf( "Failed to load sprite sheet texture!\n" ); success = false;
    }
    else
    {
        //Set ball sprite
        gBall.x = 115;
        gBall.y = 15;
        gBall.w = 20;
        gBall.h = 20;
    }

    //Load sounds
    gMiss = Mix_LoadWAV( "pong_8bit_miss.ogg" );
    if( gMiss == NULL )
    {
        printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }

    gPaddle = Mix_LoadWAV( "pong_8bit_Paddle.ogg" );
    if( gPaddle == NULL )
    {
        printf( "Failed to sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }

    gWall = Mix_LoadWAV( "pong_8bit_wall.ogg" );
    if( gWall == NULL )
    {
        printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }

    return success;
}

bool checkCollision( SDL_Rect Rect_a, SDL_Rect Rect_b )
{
    // bottom_a side outside of top_b side
    if( Rect_a.y + Rect_a.h <= Rect_b.y )
        return false;

    //top_a side outside of bottom_b
    if( Rect_a.y >= Rect_b.y + Rect_b.h )
        return false;

    //right_a outside of left_b
    if( Rect_a.x + Rect_a.w <= Rect_b.x )
        return false;

    //left_a outside of right_b
    if( Rect_a.x >= Rect_b.x + Rect_b.w )
        return false;

    //If none of the sides of a are outside of b
    return true;
}

int Ball_angle(int p_y, int b_y)
{
    int BallVel = 5 * ( ( b_y - p_y ) / 25 );
    return BallVel;
}

void close()
{
    //Free loaded images
    gPaddleTexture.free();
    gBallTexture.free();
    gBackgroundTexture.free();

    //Free global font
    TTF_CloseFont( gFont );
    gFont = NULL;

    //Free the sound effects
    Mix_FreeChunk( gPaddle );
    Mix_FreeChunk( gWall );

    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


int main()
{
    srand (time(NULL));

    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            //Main loop flag
            bool quit = false;

            //Event handler
            SDL_Event e;

            Paddle paddle;
            Ball ball;

            //While application is running
            while( !quit )
            {
                //start time
                start_time = SDL_GetTicks();
                float delta = (start_time - end_time) / 1000.0f;

                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                    }
                    if(e.type == SDL_KEYDOWN)
                    {
                        switch(e.key.keysym.sym)
                        {
                            case SDLK_ESCAPE:
                            quit = true;
                        }
                    }
                    //Input for the paddle
                    paddle.handleEvent( e );
                    ball.startEvent( e );
                }
                //Move ball
                ball.moveBall();


                //Move the paddles
                paddle.move();

                if(checkCollision(ball.cBall, paddle.pad_P1) == true )
                {
                    ball.BallXVel = ball.BALL_SPEED; //1 * ball.BallXVel; //rand() % 25;
                    //ball.BallYVel = Ball_angle(paddle.pad_P1.y, ball.cBall.y); //-1 * ball.BallYVel;
                    Mix_PlayChannel( -1, gPaddle, 0 );
                }

                if(checkCollision(ball.cBall, paddle.pad_P2) == true )
                {
                    ball.BallXVel = -ball.BALL_SPEED; //1 * ball.BallXVel; //(rand() % 25) * -1;
                    //ball.BallYVel = Ball_angle(paddle.pad_P2.y, ball.cBall.y); // -1 * ball.BallYVel;
                    Mix_PlayChannel( -1, gPaddle, 0 );
                }

                end_time = SDL_GetTicks();

                //if player 2 scores
                if( ( ball.cBall.x + ball.BALL_WIDTH ) < 0 )
                {
                    player2_score++;
                    p2_scored = true;
                    Mix_PlayChannel( -1, gMiss, 0 );
                    ball.reset();
                }

                //if player 1 scores
                if ( ball.cBall.x > SCREEN_WIDTH  )
                {
                    player1_score++;
                    p1_scored = true;
                    Mix_PlayChannel( -1, gMiss, 0 );
                    ball.reset();
                }

                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
                SDL_RenderClear( gRenderer );

                //Render background texture to screen
                gBackgroundTexture.render( 0, 0 );

                //Draw vertical line of white dots
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                for( int i = 0; i < SCREEN_HEIGHT; i += 4 )
                {
                    SDL_RenderDrawPoint( gRenderer, SCREEN_WIDTH / 2, i );
                }

                paddle.render1();
                paddle.render2();
                ball.render();

                //Update screen
                SDL_RenderPresent( gRenderer );

            }
        }
    }

    //Free resources and close SDL
    close();

    return 0;
}
