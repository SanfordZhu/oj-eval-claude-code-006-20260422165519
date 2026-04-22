#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>

int rows;
int columns;
int total_mines;
int game_state;

// Internal game state
char **map;          // 'X' for mine, '.' for safe
char **display;      // display state: '?' unvisited, '@' marked mine, 'X' visited mine, '0'-'8' visited safe
bool **visited;
bool **marked;
int visit_count;     // count of visited non-mine grids
int marked_mine_count; // count of correctly marked mines

int countAdjacentMines(int r, int c) {
  int cnt = 0;
  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr, nc = c + dc;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns && map[nr][nc] == 'X')
        ++cnt;
    }
  }
  return cnt;
}

void floodFill(int r, int c) {
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  if (visited[r][c] || marked[r][c]) return;
  visited[r][c] = true;
  ++visit_count;
  int cnt = countAdjacentMines(r, c);
  display[r][c] = '0' + cnt;
  if (cnt == 0) {
    for (int dr = -1; dr <= 1; ++dr)
      for (int dc = -1; dc <= 1; ++dc)
        if (dr != 0 || dc != 0)
          floodFill(r + dr, c + dc);
  }
}

void checkVictory() {
  // Victory when all non-mine grids are visited
  if (visit_count == rows * columns - total_mines) {
    game_state = 1;
  }
}

void InitMap() {
  std::cin >> rows >> columns;
  map = new char*[rows];
  display = new char*[rows];
  visited = new bool*[rows];
  marked = new bool*[rows];
  total_mines = 0;
  for (int i = 0; i < rows; ++i) {
    map[i] = new char[columns];
    display[i] = new char[columns];
    visited[i] = new bool[columns];
    marked[i] = new bool[columns];
    for (int j = 0; j < columns; ++j) {
      std::cin >> map[i][j];
      display[i][j] = '?';
      visited[i][j] = false;
      marked[i][j] = false;
      if (map[i][j] == 'X') ++total_mines;
    }
  }
  game_state = 0;
  visit_count = 0;
  marked_mine_count = 0;
}

void VisitBlock(int r, int c) {
  if (game_state != 0) return;
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  if (visited[r][c] || marked[r][c]) return;

  if (map[r][c] == 'X') {
    // Hit a mine
    visited[r][c] = true;
    display[r][c] = 'X';
    game_state = -1;
    return;
  }

  // Visit safe block with flood fill
  floodFill(r, c);
  checkVictory();
}

void MarkMine(int r, int c) {
  if (game_state != 0) return;
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  if (visited[r][c] || marked[r][c]) return;

  marked[r][c] = true;
  if (map[r][c] == 'X') {
    display[r][c] = '@';
    ++marked_mine_count;
  } else {
    // Marking a non-mine causes immediate failure
    display[r][c] = 'X';
    game_state = -1;
  }
}

void AutoExplore(int r, int c) {
  if (game_state != 0) return;
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  if (!visited[r][c] || marked[r][c]) return;

  int cnt = countAdjacentMines(r, c);
  int marked_around = 0;
  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr, nc = c + dc;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns && marked[nr][nc])
        ++marked_around;
    }
  }

  if (marked_around == cnt) {
    for (int dr = -1; dr <= 1; ++dr) {
      for (int dc = -1; dc <= 1; ++dc) {
        if (dr == 0 && dc == 0) continue;
        int nr = r + dr, nc = c + dc;
        if (nr >= 0 && nr < rows && nc >= 0 && nc < columns && !visited[nr][nc] && !marked[nr][nc]) {
          if (map[nr][nc] == 'X') {
            visited[nr][nc] = true;
            display[nr][nc] = 'X';
            game_state = -1;
            return;
          }
          floodFill(nr, nc);
          if (game_state != 0) return;
        }
      }
    }
    checkVictory();
  }
}

void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
    std::cout << visit_count << " " << total_mines << std::endl;
  } else {
    std::cout << "GAME OVER!" << std::endl;
    std::cout << visit_count << " " << marked_mine_count << std::endl;
  }
  exit(0);
}

void PrintMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      // On victory, all mine grids show '@'
      if (game_state == 1 && map[i][j] == 'X') {
        std::cout << '@';
      } else {
        std::cout << display[i][j];
      }
    }
    std::cout << std::endl;
  }
}

#endif
