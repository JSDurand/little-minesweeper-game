#include <iostream>
#include <vector>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
using namespace std;

const int BOARD_SIZE_W = 9;
const int BOARD_SIZE_H = 9;

const int WINDOW_WIDTH  = BOARD_SIZE_W * 64;
const int WINDOW_HEIGHT = BOARD_SIZE_H * 64;

const int number_of_bombs = BOARD_SIZE_H * BOARD_SIZE_W / 5;

enum GameState {WIN, LOSE, UNDETERMINED};

GameState game_state = UNDETERMINED;

struct Point
{
  Point() = default;
  Point(int a, int b): x(a), y(b) {};
  int x = 0;
  int y = 0;
};

bool vcontains(vector<Point> vpoint, Point p)
{
  for (vector<Point>::iterator it = vpoint.begin(); it != vpoint.end(); ++it) {
    if (it->x == p.x && it->y == p.y)
      return true;
  }

  return false;
}

vector<int> determine_hidden_board (void);

void doMouseAction (Uint32 mouse_state, int mouse_x, int mouse_y,
                    vector<int> &show_board, vector<int> &hidden_board);

int win_or_lose (vector<int> &show_board, vector<int> & hidden_board, Point mouse);

int main (void)
{
  bool reveal = false;
  
  srand(time(0));

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    cerr << "Cannot initialize SDL: " << SDL_GetError() << endl;
  
  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    cerr << "Cannot initialize image: " << SDL_GetError() << endl;

  SDL_Window *window;
  window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

  if (!window)
    cerr << "Cannot create window: " << SDL_GetError() << endl;

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer)
    cerr << "Cannot create renderer: " << SDL_GetError();

  // load image of minesweeper
  SDL_Surface *surface = IMG_Load("resource/minesweeper.png");

  if (!surface)
    cerr << "fail to load image: " << SDL_GetError() << endl;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  SDL_FreeSurface(surface);

  if (!texture)
    cerr << "fail to create texture: " << SDL_GetError() << endl;

  // win and lose textures

  SDL_Surface *win_surface = IMG_Load("resource/victory.png");

  if (!win_surface)
    cerr << "fail to load win image: " << SDL_GetError() << endl;

  SDL_Texture *win_texture = SDL_CreateTextureFromSurface(renderer, win_surface);

  SDL_FreeSurface(win_surface);

  if (!win_texture)
    cerr << "fail to create win texture: " << SDL_GetError() << endl;

  SDL_Surface *lose_surface = IMG_Load("resource/lose.png");

  if (!lose_surface)
    cerr << "fail to load lose image: " << SDL_GetError() << endl;

  SDL_Texture *lose_texture = SDL_CreateTextureFromSurface(renderer, lose_surface);

  SDL_FreeSurface(lose_surface);

  if (!lose_texture)
    cerr << "fail to create lose texture: " << SDL_GetError() << endl;

  // hidden board
  vector<int> hidden_board = determine_hidden_board();
  vector<int> show_board(BOARD_SIZE_W * BOARD_SIZE_H, 0);

  bool running = true;

  // determine materials
  int TOTAL_WIDTH, TOTAL_HEIGHT, SQUARE_WIDTH, SQUARE_HEIGHT;
  int BOARD_SQUARE_WIDTH, BOARD_SQUARE_HEIGHT;

  SDL_QueryTexture(texture, nullptr, nullptr, &TOTAL_WIDTH, &TOTAL_HEIGHT);

  BOARD_SQUARE_WIDTH  = TOTAL_WIDTH / 4;
  BOARD_SQUARE_HEIGHT = TOTAL_HEIGHT / 3;

  SQUARE_WIDTH  = TOTAL_WIDTH  / 4;
  SQUARE_HEIGHT = TOTAL_HEIGHT / 3;

  vector<SDL_Rect> materials, show_materials;

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 4; ++j) {
      SDL_Rect temp_rect;
      temp_rect.x = j * SQUARE_WIDTH;
      temp_rect.y = i * SQUARE_HEIGHT;
      temp_rect.w = SQUARE_WIDTH;
      temp_rect.h = SQUARE_HEIGHT;
      materials.push_back(temp_rect);
    }
  }

  for (int i = 0; i < BOARD_SIZE_H; ++i) {
    for (int j = 0; j < BOARD_SIZE_W; ++j) {
      SDL_Rect temp_rect;
      temp_rect.x = j * BOARD_SQUARE_WIDTH;
      temp_rect.y = i * BOARD_SQUARE_HEIGHT;
      temp_rect.w = BOARD_SQUARE_WIDTH;
      temp_rect.h = BOARD_SQUARE_HEIGHT;

      show_materials.push_back(temp_rect);
    }
  }

  while (running) {

    SDL_Event event;

    int mouse_x(-1), mouse_y(-1);

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      default:
        break;
      }

      Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);

      mouse_x = (int)(mouse_x / BOARD_SQUARE_WIDTH);
      mouse_y = (int)(mouse_y / BOARD_SQUARE_HEIGHT);

      doMouseAction(mouse_state, mouse_x, mouse_y, show_board, hidden_board);

      const Uint8 *state = SDL_GetKeyboardState(nullptr);

      if (state[SDL_SCANCODE_ESCAPE]) 
        running = false;
      else if (state[SDL_SCANCODE_A]) 
        running = false;
      else if (state[SDL_SCANCODE_SPACE]) {
        game_state = UNDETERMINED;
        reveal = false;
        hidden_board = determine_hidden_board();
        for (vector<int>::size_type i = 0; i < show_board.size(); ++i)
          show_board[i] = 0;
      } else if (state[SDL_SCANCODE_RETURN])
        reveal = !reveal;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    int win_or_lose_or_nothing = win_or_lose(show_board, hidden_board, Point(mouse_x, mouse_y));

    if (win_or_lose_or_nothing == 1 && game_state == UNDETERMINED)
      game_state = WIN;
    else if (win_or_lose_or_nothing == -1 && game_state == UNDETERMINED)
      game_state = LOSE;

    if (reveal)
      for (vector<SDL_Rect>::size_type i = 0; i < show_materials.size(); ++i) 
        SDL_RenderCopy(renderer, texture, &materials[hidden_board[i]], &show_materials[i]);
    else if (game_state == UNDETERMINED) 
      for (vector<SDL_Rect>::size_type i = 0; i < show_materials.size(); ++i) 
        SDL_RenderCopy(renderer, texture, &materials[show_board[i]], &show_materials[i]);
    else if (game_state == WIN) 
      SDL_RenderCopy(renderer, win_texture, nullptr, nullptr);
    else {
      // cout << "result: " << game_state << endl;
      SDL_RenderCopy(renderer, lose_texture, nullptr, nullptr);
    }

    SDL_RenderPresent(renderer);
  }

  IMG_Quit();
  SDL_DestroyTexture(texture);
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
  
  return 0;
}

vector<int> determine_hidden_board (void)
{
  vector<int> hidden_board;

  vector<Point> bomb_locations;

  // determine bomb locations
  for (int i = 0; i < number_of_bombs; ++i) {
    bool found = false;
    int count  = 0;
    Point p;

    while (!found && count < 5) {
      ++count;

      int x = rand() % BOARD_SIZE_H;
      int y = rand() % BOARD_SIZE_W;
      p     = Point(x, y);

      if (!vcontains(bomb_locations, p))
        found = true;
    }

    if (found) {
      bomb_locations.push_back(p);
    } else {
      int x = 0, y = 0;
      Point p = Point(x, y);
      while (vcontains(bomb_locations, p)) {
        if (x < BOARD_SIZE_W - 1) 
          ++x;
        else       
          ++y;

        p = Point(x, y);
      }

      bomb_locations.push_back(p);
    }
  }

  // determine the hidden board
  int bomb_count = 0;
  for (int i = 0; i < BOARD_SIZE_H; ++i) {
    for (int j = 0; j < BOARD_SIZE_W; ++j) {
      if (vcontains(bomb_locations, Point(i, j))) {
        hidden_board.push_back(2); // a bomb
      } else {
        bomb_count = 3;

        for (int k = -1; k < 2; ++k) {
          for (int l = -1; l < 2; ++l) {
            if (k == 0 && l == 0)
              continue;

            if (vcontains(bomb_locations, Point(i + k, j + l)))
              ++bomb_count;
          }
        }
        hidden_board.push_back(bomb_count);
      }
    }
  }

  return hidden_board;
}

void doMouseAction (Uint32 mouse_state, int mouse_x, int mouse_y,
                    vector<int> &show_board, vector<int> &hidden_board)
{
  vector<int>::size_type index = mouse_y * BOARD_SIZE_W + mouse_x;

  if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
    if (show_board[index] == 0 || show_board[index] == 1) {
      if (hidden_board[index] == 2) {
        for (vector<int>::size_type i = 0; i < show_board.size(); ++i) {
          show_board[i] = hidden_board[i];
        }
      } else if (hidden_board[index] != 3) {
        show_board[index] = hidden_board[index];
      } else {
        show_board[index] = hidden_board[index];
        
        // determine the vector of neighbouring empty cells

        vector<Point> doing, todo, done;

        doing.push_back(Point(mouse_x, mouse_y));

        while (doing.begin() != doing.end()) {
          for (auto p : doing) {
            done.push_back(p);
            
            int px = p.x, py = p.y;

            for (int i = -1; i < 2; ++i) {
              for (int j = -1; j < 2; ++j) {
                if (i == 0 && j == 0)
                  continue;
                if (px + i < 0 || px + i >= BOARD_SIZE_W ||
                    py + j < 0 || py + j >= BOARD_SIZE_H)
                  continue;

                int neighbour = (py + j) * BOARD_SIZE_W + px + i;
                show_board[neighbour] = hidden_board[neighbour];

                if (hidden_board[neighbour] == 3 && !vcontains(done, Point(px + i, py + j)))
                  todo.push_back(Point(px + i, py + j));
              }
            }
          }

          doing = todo;
          todo  = vector<Point>();
        }

      }
    }
  } else if (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
    if (show_board[index] == 0)
      show_board[index] = 1;
  }
}

// 0 => not finished yet; 1 => win; -1 => lose
int win_or_lose (vector<int> &show_board, vector<int> & hidden_board, Point mouse)
{
  for (vector<int>::size_type i = 0; i < show_board.size(); ++i) {
    if (show_board[i] != hidden_board[i] && show_board[i] != 1) {
      return 0;
    }
  }

  vector<int>::size_type index = mouse.y * BOARD_SIZE_W + mouse.x;

  if (hidden_board[index] == 2 && show_board[index] != 1) {
    return -1;
  } else {
    return 1;
  }
}
