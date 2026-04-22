#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <cstring>
#include <algorithm>
#include <climits>
#include <queue>
#include <cmath>
#include <map>

extern int rows;
extern int columns;
extern int total_mines;

void Execute(int r, int c, int type);

char **known_map;
bool **is_mine;
bool **is_safe;
bool **cell_visited;
int flagged_count;

const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

void InitGame() {
  known_map = new char*[rows];
  is_mine = new bool*[rows];
  is_safe = new bool*[rows];
  cell_visited = new bool*[rows];
  for (int i = 0; i < rows; ++i) {
    known_map[i] = new char[columns];
    is_mine[i] = new bool[columns];
    is_safe[i] = new bool[columns];
    cell_visited[i] = new bool[columns];
    for (int j = 0; j < columns; ++j) {
      known_map[i][j] = '?';
      is_mine[i][j] = false;
      is_safe[i][j] = false;
      cell_visited[i][j] = false;
    }
  }
  flagged_count = 0;

  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

void ReadMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch;
      std::cin >> ch;
      known_map[i][j] = ch;
      if (ch >= '0' && ch <= '8') {
        cell_visited[i][j] = true;
        is_safe[i][j] = true;
      } else if (ch == '@') {
        is_mine[i][j] = true;
        is_safe[i][j] = true;
      } else if (ch == 'X') {
        cell_visited[i][j] = true;
      }
    }
  }
}

int countUnknown(int r, int c) {
  int cnt = 0;
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (nr >= 0 && nr < rows && nc >= 0 && nc < columns && !is_safe[nr][nc])
      ++cnt;
  }
  return cnt;
}

int countFlagged(int r, int c) {
  int cnt = 0;
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (nr >= 0 && nr < rows && nc >= 0 && nc < columns && is_mine[nr][nc])
      ++cnt;
  }
  return cnt;
}

void getUnknownNeighbors(int r, int c, std::vector<std::pair<int,int>>& out) {
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (nr >= 0 && nr < rows && nc >= 0 && nc < columns && !is_safe[nr][nc])
      out.push_back({nr, nc});
  }
}

bool applyBasicRules() {
  bool acted = false;

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (!cell_visited[i][j]) continue;
      int num = known_map[i][j] - '0';
      if (num < 0 || num > 8) continue;

      int unknown = countUnknown(i, j);
      int flagged = countFlagged(i, j);
      int remaining = num - flagged;

      if (unknown == 0) continue;

      if (remaining == unknown) {
        std::vector<std::pair<int,int>> unknowns;
        getUnknownNeighbors(i, j, unknowns);
        for (auto& p : unknowns) {
          if (!is_mine[p.first][p.second]) {
            is_mine[p.first][p.second] = true;
            is_safe[p.first][p.second] = true;
            ++flagged_count;
            acted = true;
          }
        }
      } else if (remaining == 0) {
        std::vector<std::pair<int,int>> unknowns;
        getUnknownNeighbors(i, j, unknowns);
        for (auto& p : unknowns) {
          if (!is_safe[p.first][p.second]) {
            is_safe[p.first][p.second] = true;
            acted = true;
          }
        }
      }
    }
  }
  return acted;
}

bool applyAdvancedRules() {
  struct Constraint {
    std::vector<std::pair<int,int>> cells;
    int mines;
  };

  std::vector<Constraint> constraints;

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (!cell_visited[i][j]) continue;
      int num = known_map[i][j] - '0';
      if (num < 0 || num > 8) continue;
      int flagged = countFlagged(i, j);
      int remaining = num - flagged;
      if (remaining < 0) continue;

      Constraint c;
      getUnknownNeighbors(i, j, c.cells);
      c.mines = remaining;
      if (!c.cells.empty()) {
        constraints.push_back(c);
      }
    }
  }

  bool acted = false;

  for (size_t a = 0; a < constraints.size(); ++a) {
    for (size_t b = a + 1; b < constraints.size(); ++b) {
      auto& ca = constraints[a];
      auto& cb = constraints[b];

      bool ca_sub_cb = true;
      for (auto& cell : ca.cells) {
        bool found = false;
        for (auto& cell2 : cb.cells) {
          if (cell.first == cell2.first && cell.second == cell2.second) {
            found = true;
            break;
          }
        }
        if (!found) { ca_sub_cb = false; break; }
      }

      if (ca_sub_cb) {
        std::vector<std::pair<int,int>> diff;
        for (auto& cell : cb.cells) {
          bool found = false;
          for (auto& cell2 : ca.cells) {
            if (cell.first == cell2.first && cell.second == cell2.second) {
              found = true;
              break;
            }
          }
          if (!found) diff.push_back(cell);
        }
        int mine_diff = cb.mines - ca.mines;
        if (mine_diff == 0 && !diff.empty()) {
          for (auto& p : diff) {
            if (!is_safe[p.first][p.second]) {
              is_safe[p.first][p.second] = true;
              acted = true;
            }
          }
        } else if (mine_diff == (int)diff.size() && !diff.empty()) {
          for (auto& p : diff) {
            if (!is_mine[p.first][p.second]) {
              is_mine[p.first][p.second] = true;
              is_safe[p.first][p.second] = true;
              ++flagged_count;
              acted = true;
            }
          }
        }
        continue;
      }

      bool cb_sub_ca = true;
      for (auto& cell : cb.cells) {
        bool found = false;
        for (auto& cell2 : ca.cells) {
          if (cell.first == cell2.first && cell.second == cell2.second) {
            found = true;
            break;
          }
        }
        if (!found) { cb_sub_ca = false; break; }
      }

      if (cb_sub_ca) {
        std::vector<std::pair<int,int>> diff;
        for (auto& cell : ca.cells) {
          bool found = false;
          for (auto& cell2 : cb.cells) {
            if (cell.first == cell2.first && cell.second == cell2.second) {
              found = true;
              break;
            }
          }
          if (!found) diff.push_back(cell);
        }
        int mine_diff = ca.mines - cb.mines;
        if (mine_diff == 0 && !diff.empty()) {
          for (auto& p : diff) {
            if (!is_safe[p.first][p.second]) {
              is_safe[p.first][p.second] = true;
              acted = true;
            }
          }
        } else if (mine_diff == (int)diff.size() && !diff.empty()) {
          for (auto& p : diff) {
            if (!is_mine[p.first][p.second]) {
              is_mine[p.first][p.second] = true;
              is_safe[p.first][p.second] = true;
              ++flagged_count;
              acted = true;
            }
          }
        }
      }
    }
  }

  return acted;
}

int countTotalUnknown() {
  int cnt = 0;
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < columns; ++j)
      if (!is_safe[i][j]) ++cnt;
  return cnt;
}

bool getSafeUnvisited(int& r, int& c) {
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < columns; ++j)
      if (is_safe[i][j] && !is_mine[i][j] && !cell_visited[i][j]) {
        r = i; c = j;
        return true;
      }
  return false;
}

bool getMineToMark(int& r, int& c) {
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < columns; ++j)
      if (is_mine[i][j] && known_map[i][j] != '@') {
        r = i; c = j;
        return true;
      }
  return false;
}

bool getAutoExploreTarget(int& r, int& c) {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (!cell_visited[i][j]) continue;
      int num = known_map[i][j] - '0';
      if (num < 0 || num > 8) continue;
      int flagged = countFlagged(i, j);
      int unknown = countUnknown(i, j);
      if (flagged > 0 && flagged == num && unknown > 0) {
        r = i; c = j;
        return true;
      }
    }
  }
  return false;
}

// Find the best cell to guess using constraint-based probability
void findBestGuess(int& r, int& c) {
  int remaining_mines = total_mines - flagged_count;
  int unknown_cells = countTotalUnknown();
  double global_prob = unknown_cells > 0 ? (double)remaining_mines / unknown_cells : 0.0;

  // Build constraints
  struct Constraint {
    std::vector<std::pair<int,int>> cells;
    int mines;
  };

  std::vector<Constraint> constraints;

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (!cell_visited[i][j]) continue;
      int num = known_map[i][j] - '0';
      if (num < 0 || num > 8) continue;
      int flagged = countFlagged(i, j);
      int remaining = num - flagged;
      if (remaining < 0) continue;

      Constraint c;
      getUnknownNeighbors(i, j, c.cells);
      c.mines = remaining;
      if (!c.cells.empty()) {
        constraints.push_back(c);
      }
    }
  }

  // Compute per-cell probability
  double best_score = -1e9;
  int best_r = -1, best_c = -1;

  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (is_safe[i][j]) continue;

      // Check if adjacent to visited
      bool adjacent = false;
      for (int k = 0; k < 8; ++k) {
        int ni = i + dr[k], nj = j + dc[k];
        if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && cell_visited[ni][nj]) {
          adjacent = true;
          break;
        }
      }

      double p;
      if (adjacent) {
        // Compute probability from adjacent constraints
        double min_prob = 1.0;
        for (auto& c : constraints) {
          bool found = false;
          for (auto& cell : c.cells) {
            if (cell.first == i && cell.second == j) {
              found = true;
              break;
            }
          }
          if (!found) continue;
          double local_prob = (double)c.mines / c.cells.size();
          if (local_prob < min_prob) min_prob = local_prob;
        }
        p = min_prob;
      } else {
        p = global_prob;
      }

      double score = -p;
      if (adjacent) score += 1e-9;

      if (score > best_score) {
        best_score = score;
        best_r = i;
        best_c = j;
      }
    }
  }

  if (best_r >= 0) {
    r = best_r;
    c = best_c;
  } else {
    for (int i = 0; i < rows; ++i)
      for (int j = 0; j < columns; ++j)
        if (!is_safe[i][j]) { r = i; c = j; return; }
  }
}

void Decide() {
  extern int game_state;
  if (game_state != 0) return;

  while (applyBasicRules() || applyAdvancedRules()) {
    int mr, mc;
    if (getMineToMark(mr, mc)) { Execute(mr, mc, 1); return; }
    if (getAutoExploreTarget(mr, mc)) { Execute(mr, mc, 2); return; }
    if (getSafeUnvisited(mr, mc)) { Execute(mr, mc, 0); return; }
  }

  int ar, ac;
  if (getAutoExploreTarget(ar, ac)) { Execute(ar, ac, 2); return; }

  int mr, mc;
  if (getMineToMark(mr, mc)) { Execute(mr, mc, 1); return; }
  if (getSafeUnvisited(mr, mc)) { Execute(mr, mc, 0); return; }

  int gr, gc;
  findBestGuess(gr, gc);
  Execute(gr, gc, 0);
}

#endif
