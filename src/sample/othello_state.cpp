#include "othello_state.hpp"

const std::array<OthelloState::bitboard, 64> OthelloState::kSquare{
    0x80'00'00'00'00'00'00'00, 0x40'00'00'00'00'00'00'00,
    0x20'00'00'00'00'00'00'00, 0x10'00'00'00'00'00'00'00,
    0x08'00'00'00'00'00'00'00, 0x04'00'00'00'00'00'00'00,
    0x02'00'00'00'00'00'00'00, 0x01'00'00'00'00'00'00'00,
    0x00'80'00'00'00'00'00'00, 0x00'40'00'00'00'00'00'00,
    0x00'20'00'00'00'00'00'00, 0x00'10'00'00'00'00'00'00,
    0x00'08'00'00'00'00'00'00, 0x00'04'00'00'00'00'00'00,
    0x00'02'00'00'00'00'00'00, 0x00'01'00'00'00'00'00'00,
    0x00'00'80'00'00'00'00'00, 0x00'00'40'00'00'00'00'00,
    0x00'00'20'00'00'00'00'00, 0x00'00'10'00'00'00'00'00,
    0x00'00'08'00'00'00'00'00, 0x00'00'04'00'00'00'00'00,
    0x00'00'02'00'00'00'00'00, 0x00'00'01'00'00'00'00'00,
    0x00'00'00'80'00'00'00'00, 0x00'00'00'40'00'00'00'00,
    0x00'00'00'20'00'00'00'00, 0x00'00'00'10'00'00'00'00,
    0x00'00'00'08'00'00'00'00, 0x00'00'00'04'00'00'00'00,
    0x00'00'00'02'00'00'00'00, 0x00'00'00'01'00'00'00'00,
    0x00'00'00'00'80'00'00'00, 0x00'00'00'00'40'00'00'00,
    0x00'00'00'00'20'00'00'00, 0x00'00'00'00'10'00'00'00,
    0x00'00'00'00'08'00'00'00, 0x00'00'00'00'04'00'00'00,
    0x00'00'00'00'02'00'00'00, 0x00'00'00'00'01'00'00'00,
    0x00'00'00'00'00'80'00'00, 0x00'00'00'00'00'40'00'00,
    0x00'00'00'00'00'20'00'00, 0x00'00'00'00'00'10'00'00,
    0x00'00'00'00'00'08'00'00, 0x00'00'00'00'00'04'00'00,
    0x00'00'00'00'00'02'00'00, 0x00'00'00'00'00'01'00'00,
    0x00'00'00'00'00'00'80'00, 0x00'00'00'00'00'00'40'00,
    0x00'00'00'00'00'00'20'00, 0x00'00'00'00'00'00'10'00,
    0x00'00'00'00'00'00'08'00, 0x00'00'00'00'00'00'04'00,
    0x00'00'00'00'00'00'02'00, 0x00'00'00'00'00'00'01'00,
    0x00'00'00'00'00'00'00'80, 0x00'00'00'00'00'00'00'40,
    0x00'00'00'00'00'00'00'20, 0x00'00'00'00'00'00'00'10,
    0x00'00'00'00'00'00'00'08, 0x00'00'00'00'00'00'00'04,
    0x00'00'00'00'00'00'00'02, 0x00'00'00'00'00'00'00'01};

OthelloState OthelloState::next(const coord& action) const {
  const bitboard put{OthelloState::coord2Bit(action)};

  bitboard my_board{};
  bitboard opponent_board{};
  if (this->cur_turn_ == OthelloState::kBlackTurn) {
    my_board = this->black_board_;
    opponent_board = this->white_board_;
  } else {
    my_board = this->white_board_;
    opponent_board = this->black_board_;
  }
  bitboard reversed_squares{};

  /* putが置かれることで反転される箇所を8方向に走査する。 */
  for (int i = 0; i < 8; i++) {
    bitboard reversed_line{};  // ある1方向において反転された箇所の全体。
    bitboard cur_square{this->nextSquare(put, i)};  // 反転可能性を見るマス。
    while ((cur_square != (bitboard)0) &&
           ((cur_square & opponent_board) != (bitboard)0)) {
      reversed_line |= cur_square;
      cur_square = nextSquare(cur_square, i);
    }
    /* 反転できそうなマスの列(reversed_line)の先に自分の石があれば、反転できる。 */
    if ((cur_square & my_board) != (bitboard)0) {
      reversed_squares |= reversed_line;
    }
  }

  my_board ^= (put | reversed_squares);
  opponent_board ^= reversed_squares;

  OthelloState result{*this};
  if (this->cur_turn_ == OthelloState::kBlackTurn) {
    result.cur_turn_ = kWhiteTurn;
    result.black_board_ = my_board;
    result.white_board_ = opponent_board;
  } else {
    result.cur_turn_ = kBlackTurn;
    result.black_board_ = opponent_board;
    result.white_board_ = my_board;
  }

  if (result.isPass()) {
    result.cur_turn_ = (result.cur_turn_ == OthelloState::kBlackTurn) ? OthelloState::kWhiteTurn : OthelloState::kBlackTurn;
  }

  return result;
}

std::vector<OthelloState::coord> OthelloState::legalActions() const {
  std::vector<coord> result{};
  bitboard tmp{this->legalBoard()};
  for (int i = 7; i >= 0; i--) {
    for (int j = 7; j >= 0; j--) {
      if (tmp % 2 == 1) {
        result.push_back(coord(j, i));
      }
      tmp >>= 1;
    }
  }
  return result;
}

bool OthelloState::isFinished() const {
  /* 現在手番の合法手全体。 */
  const bitboard my_legal_board{this->legalBoard()};

  /* 次手番の合法手全体。 */
  OthelloState next_state{OthelloState(*this)};
  next_state.cur_turn_ = (this->cur_turn_ == OthelloState::kBlackTurn) ? kWhiteTurn : kBlackTurn;
  const bitboard opponent_legal_board{next_state.legalBoard()};

  /* 現在手番だけで合法手がなければ、パス。次手番でも合法手がなければ、終局。 */
  return my_legal_board == (bitboard)0 && opponent_legal_board == (bitboard)0;
}

int OthelloState::getScore(const int player_num) const {
  if (!this->isFinished()) {
    return 0;
  }

  if (OthelloState::count(this->black_board_) ==
      OthelloState::count(this->white_board_)) {
    return 0;
  }

  if (player_num == OthelloState::kBlackTurn) {
    return (OthelloState::count(this->black_board_) > OthelloState::count(this->white_board_)) ? 1 : 0;
  } else {
    return (OthelloState::count(this->black_board_) < OthelloState::count(this->white_board_)) ? 1 : 0;
  }
}

std::string OthelloState::board2String() const {
  std::vector<int> squares{};
  bitboard tmp_black{this->black_board_};
  bitboard tmp_white{this->white_board_};

  std::string result{};

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (tmp_black % 2 == 1) {
        squares.push_back(1);
      } else if (tmp_white % 2 == 1) {
        squares.push_back(-1);
      } else {
        squares.push_back(0);
      }
      tmp_black >>= 1;
      tmp_white >>= 1;
    }
  }

  result += " ABCDEFGH\n";
  for (int i = 7; i >= 0; i--) {
    result += std::to_string(8 - i);
    for (int j = 7; j >= 0; j--) {
      switch (squares.at(8 * i + j)) {
        case 1:
          result += "X";
          break;
        case -1:
          result += "O";
          break;
        default:
          result += "-";
          break;
      }
    }
    result += "\n";
  }
  return result;
};

OthelloState::coord OthelloState::str2Coord(std::string str) {
  if (str.size() != 2) {
    return {-1, -1};
  }

  coord c{};
  if (str.at(0) == 'a') {
    c.first = 0;
  } else if (str.at(0) == 'b') {
    c.first = 1;
  } else if (str.at(0) == 'c') {
    c.first = 2;
  } else if (str.at(0) == 'd') {
    c.first = 3;
  } else if (str.at(0) == 'e') {
    c.first = 4;
  } else if (str.at(0) == 'f') {
    c.first = 5;
  } else if (str.at(0) == 'g') {
    c.first = 6;
  } else if (str.at(0) == 'h') {
    c.first = 7;
  } else {
    return {-1, -1};
  }
  if (str.at(1) == '1') {
    c.second = 0;
  } else if (str.at(1) == '2') {
    c.second = 1;
  } else if (str.at(1) == '3') {
    c.second = 2;
  } else if (str.at(1) == '4') {
    c.second = 3;
  } else if (str.at(1) == '5') {
    c.second = 4;
  } else if (str.at(1) == '6') {
    c.second = 5;
  } else if (str.at(1) == '7') {
    c.second = 6;
  } else if (str.at(1) == '8') {
    c.second = 7;
  } else {
    return {-1, -1};
  }

  return c;
}

std::string OthelloState::coord2Str(OthelloState::coord c) {
  std::string s{};
  if (c.first == 0) {
    s.push_back('a');
  } else if (c.first == 1) {
    s.push_back('b');
  } else if (c.first == 2) {
    s.push_back('c');
  } else if (c.first == 3) {
    s.push_back('d');
  } else if (c.first == 4) {
    s.push_back('e');
  } else if (c.first == 5) {
    s.push_back('f');
  } else if (c.first == 6) {
    s.push_back('g');
  } else if (c.first == 7) {
    s.push_back('h');
  } else {
    return "";
  }
  if (c.second == 0) {
    s.push_back('1');
  } else if (c.second == 1) {
    s.push_back('2');
  } else if (c.second == 2) {
    s.push_back('3');
  } else if (c.second == 3) {
    s.push_back('4');
  } else if (c.second == 4) {
    s.push_back('5');
  } else if (c.second == 5) {
    s.push_back('6');
  } else if (c.second == 6) {
    s.push_back('7');
  } else if (c.second == 7) {
    s.push_back('8');
  } else {
    return "";
  }

  return s;
}

OthelloState::bitboard OthelloState::legalBoard() const {
  bitboard my_board{};
  bitboard opponent_board{};
  if (this->cur_turn_ == OthelloState::kBlackTurn) {
    my_board = this->black_board_;
    opponent_board = this->white_board_;
  } else {
    my_board = this->white_board_;
    opponent_board = this->black_board_;
  }

  /* 左右方向・上下方向・斜め方向にそれぞれ挟めるマス全体。端は挟めないので積をとって除いている。
   */
  const bitboard horizontal_sandwichable_squares = opponent_board & 0x7e7e7e7e7e7e7e7e;
  const bitboard vertical_sandwichable_squares = opponent_board & 0x00FFFFFFFFFFFF00;
  const bitboard diagonal_sandwichable_squares = opponent_board & 0x007e7e7e7e7e7e00;

  /* 空のマス全体。 */
  const bitboard blank_squares = ~(my_board | opponent_board);

  bitboard tmp_board{};
  bitboard result{};

  /* 左方向に置ける場所を探索。 */
  /* 自分のマスの左側に挟めるマスが連続して存在する限り、そこにbitを立てていく。
   */
  tmp_board = horizontal_sandwichable_squares & (my_board << 1);
  for (int i = 0; i < 5; i++) {
    tmp_board |= horizontal_sandwichable_squares & (tmp_board << 1);
  }
  /* 空のマスと(挟めるマス全体とその1つ左のマス全体)の積をとることで、石を置けるマスを求めた。
   */
  result |= blank_squares & (tmp_board << 1);

  /* 右方向。 */
  tmp_board = horizontal_sandwichable_squares & (my_board >> 1);
  for (int i = 0; i < 5; i++) {
    tmp_board |= horizontal_sandwichable_squares & (tmp_board >> 1);
  }
  result |= blank_squares & (tmp_board >> 1);

  /* 上方向。 */
  tmp_board = vertical_sandwichable_squares & (my_board << 8);
  for (int i = 0; i < 5; i++) {
    tmp_board |= vertical_sandwichable_squares & (tmp_board << 8);
  }
  result |= blank_squares & (tmp_board << 8);

  /* 下方向。 */
  tmp_board = vertical_sandwichable_squares & (my_board >> 8);
  for (int i = 0; i < 5; i++) {
    tmp_board |= vertical_sandwichable_squares & (tmp_board >> 8);
  }
  result |= blank_squares & (tmp_board >> 8);

  /* 右上方向。 */
  tmp_board = diagonal_sandwichable_squares & (my_board << 7);
  for (int i = 0; i < 5; i++) {
    tmp_board |= diagonal_sandwichable_squares & (tmp_board << 7);
  }
  result |= blank_squares & (tmp_board << 7);

  /* 左上方向。 */
  tmp_board = diagonal_sandwichable_squares & (my_board << 9);
  for (int i = 0; i < 5; i++) {
    tmp_board |= diagonal_sandwichable_squares & (tmp_board << 9);
  }
  result |= blank_squares & (tmp_board << 9);

  /* 右下方向。 */
  tmp_board = diagonal_sandwichable_squares & (my_board >> 9);
  for (int i = 0; i < 5; i++) {
    tmp_board |= diagonal_sandwichable_squares & (tmp_board >> 9);
  }
  result |= blank_squares & (tmp_board >> 9);

  /* 左下方向。 */
  tmp_board = diagonal_sandwichable_squares & (my_board >> 7);
  for (int i = 0; i < 5; i++) {
    tmp_board |= diagonal_sandwichable_squares & (tmp_board >> 7);
  }
  result |= blank_squares & (tmp_board >> 7);

  return result;
}

OthelloState::bitboard OthelloState::nextSquare(const bitboard square, const int direction) const {
  /* 8方向について、1つ先に移動できればその場所に1が立ったbit表現を返す。 */
  switch (direction) {
    case 0:  // 上。下には移動できないので、一番下はbitが立っていない。
      return (square << 8) & 0xffffffffffffff00;
    case 1:  // 右上。
      return (square << 7) & 0x7f7f7f7f7f7f7f00;
    case 2:  // 右。
      return (square >> 1) & 0x7f7f7f7f7f7f7f7f;
    case 3:  // 右下。
      return (square >> 9) & 0x007f7f7f7f7f7f7f;
    case 4:  // 下。
      return (square >> 8) & 0x00ffffffffffffff;
    case 5:  // 左下。
      return (square >> 7) & 0x00fefefefefefefe;
    case 6:  // 左。
      return (square << 1) & 0xfefefefefefefefe;
    case 7:  // 左上。
      return (square << 9) & 0xfefefefefefefe00;
    default:
      return 0;
  }
}

bool OthelloState::isPass() const {
  /* 現在手番の合法手全体。 */
  const bitboard my_legal_board{this->legalBoard()};

  /* 次手番の合法手全体。 */
  OthelloState next_state{OthelloState(*this)};
  next_state.cur_turn_ = (this->cur_turn_ == OthelloState::kBlackTurn) ? kWhiteTurn : kBlackTurn;
  const bitboard opponent_legal_board{next_state.legalBoard()};

  /* 現在手番だけで合法手がなければ、パス。次手番でも合法手がなければ、終局。 */
  return my_legal_board == (bitboard)0 && opponent_legal_board != (bitboard)0;
}

std::ostream& operator<<(std::ostream& os, const OthelloState& src) {
  os << "## OthelloState" << std::endl;
  os << "# Table" << std::endl;
  os << src.board2String();
  os << "# 現在のプレイヤ" << std::endl;
    os << ((src.cur_turn_ == OthelloState::kBlackTurn) ?
        "黒" : "白") << std::endl;
  return os;
}
