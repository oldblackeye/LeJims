#include "ludo_player_ann.h"
#include <random>



ludo_player_ann::ludo_player_ann():
    pos_start_of_turn(16),
    pos_end_of_turn(16),
    dice_roll(0),
    rd(),
    gen(rd())
{
}

ludo_player_ann::ludo_player_ann(int playerType):
    pos_start_of_turn(16),
    pos_end_of_turn(16),
    dice_roll(0)
{
    player_type = playerType;
}


int ludo_player_ann::make_random_decision(){
    std::vector<int> valid_moves;
    if(dice_roll == 6){
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]<0){
                valid_moves.push_back(i);
            }
        }
    }
    for(int i = 0; i < 4; ++i){
        if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
            valid_moves.push_back(i);
        }
    }
    if(valid_moves.size()==0){
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i] != 99){
                valid_moves.push_back(i);
            }
        }
    }
    std::uniform_int_distribution<> piece(0, valid_moves.size()-1);
    int select = piece(gen);
    return valid_moves[select];
}

int ludo_player_ann::make_decision(){
    if(dice_roll == 6){
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]<0){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
                return i;
            }
        }
    } else {
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){ //maybe they are all locked in
            if(pos_start_of_turn[i]<0){
                return i;
            }
        }
    }
    return -1;
}

int ludo_player_ann::make_defensive_decision()
{
    if(dice_roll == 6)
    {
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]<0){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){
            if(move_global_safe(i,dice_roll) && pos_start_of_turn[i] != 99){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i)
        {
            if(move_local_safe(pos_start_of_turn[i],dice_roll) && pos_start_of_turn[i] != 99)
            {
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
                return i;
            }
        }
    }
    else {
        for(int i = 0; i < 4; ++i){
            if(move_global_safe(i,dice_roll) && pos_start_of_turn[i] != 99){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i)
        {
            if(move_local_safe(pos_start_of_turn[i],dice_roll) && pos_start_of_turn[i] != 99)
            {
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
                return i;
            }
        }
        for(int i = 0; i < 4; ++i){ //maybe they are all locked in
            if(pos_start_of_turn[i]<0){
                return i;
            }
        }
    }
    return -1;
}

std::vector < int > ludo_player_ann::sorted_index(fann_type* calc_out){
  double fourth_largest = 90;
  double third_largest = 0;
  double second_largest = 0;
  double largest = 0;
  int fourth_largest_index = -99;
  int third_largest_index = -99;
  int second_largest_index = -99;
  int largest_index = -99;
  //Finding Largest
  for (int i = 0; i < 4; ++i)
    if (calc_out[i]>largest){
      largest = calc_out[i];
      largest_index = i;
    }
  //finding second largset
  for (int i = 0; i < 4; ++i)
   if (calc_out[i]>second_largest){
      if (calc_out[i] == largest)
            continue;
      second_largest = calc_out[i];
      second_largest_index = i;
   }
  //finding third largset
  for (int i = 0; i < 4; ++i)
   if (calc_out[i]>third_largest){
      if (calc_out[i] == largest || calc_out[i] == second_largest)
            continue;
      third_largest = calc_out[i];
      third_largest_index = i;
   }
  //finding fourth largset
  for (int i = 0; i < 4; ++i){
    if ( calc_out[i] < fourth_largest ){
      fourth_largest = calc_out[i];
      fourth_largest_index = i;
    }
  }

   return std::vector<int>{largest_index,second_largest_index,third_largest_index,fourth_largest_index};
}


int ludo_player_ann::make_ann_decision(){
    fann_type *calc_out;
    struct fann *ann = fann_create_from_file(TRAINED_NETWORK_PATH);
    fann_type ann_input[56];


    for(int i = 0; i < 4; i++)
    {
        ann_input[0+(i*14)] = is_on_home_stretch(pos_start_of_turn.at(i));
        ann_input[1+(i*14)] = can_move_to_home_stretch(pos_start_of_turn.at(i),dice_roll);
        ann_input[2+(i*14)] = can_kill_enemy(dice_roll,i);
        ann_input[3+(i*14)] = move_star(pos_start_of_turn.at(i),dice_roll);
        ann_input[4+(i*14)] = can_complete(pos_start_of_turn.at(i),dice_roll);
        ann_input[5+(i*14)] = is_complete(pos_start_of_turn.at(i));
        ann_input[6+(i*14)] = move_out_home(pos_start_of_turn.at(i),dice_roll);
        ann_input[7+(i*14)] = is_global_safe(i);
        ann_input[8+(i*14)] = move_global_safe(i,dice_roll);
        ann_input[9+(i*14)] = is_local_safe(pos_start_of_turn.at(i));
        ann_input[10+(i*14)] = move_local_safe(pos_start_of_turn.at(i),dice_roll);
        ann_input[11+(i*14)] = is_enemy_start(pos_start_of_turn.at(i));
        ann_input[12+(i*14)] = move_enemy_start(pos_start_of_turn.at(i),dice_roll);
        ann_input[13+(i*14)] = is_home(pos_start_of_turn.at(i));
    }

    calc_out = fann_run(ann, ann_input);

    std::vector<int> indx_Sorted = sorted_index(calc_out);


    for(int i = 0; i < 4 ; i++){
        if(pos_start_of_turn[i] != 99){
            if(pos_start_of_turn[i] >=0 && dice_roll != 6)
                return indx_Sorted[i];
        }
    }

    fann_destroy(ann);

    return 1;
}


void ludo_player_ann::start_turn(positions_and_dice relative){
    pos_start_of_turn = relative.pos;
    dice_roll = relative.dice;
    int decision;
    if(player_type == AGGRESIVE)
        decision = make_aggressive_decision(); //make aggo
    else if(player_type == DEFENSIVE)
        decision = make_defensive_decision(); //make pacifist
    else if(player_type == RANDOM)
        decision = make_random_decision();
    else if(player_type == NORMAL)
        decision = make_decision();
    else if(player_type == ANN)
        decision = make_ann_decision();

    for(int i = 0; i < 4; i++)
    {
        input.push_back(is_on_home_stretch(pos_start_of_turn.at(i)));
        input.push_back(can_move_to_home_stretch(pos_start_of_turn.at(i),dice_roll));
        input.push_back(can_kill_enemy(dice_roll,i));
        input.push_back(move_star(pos_start_of_turn.at(i),dice_roll));
        input.push_back(can_complete(pos_start_of_turn.at(i),dice_roll));
        input.push_back(is_complete(pos_start_of_turn.at(i)));
        input.push_back(move_out_home(pos_start_of_turn.at(i),dice_roll));
        input.push_back(is_global_safe(i));
        input.push_back(move_global_safe(i,dice_roll));
        input.push_back(is_local_safe(pos_start_of_turn.at(i)));
        input.push_back(move_local_safe(pos_start_of_turn.at(i),dice_roll));
        input.push_back(is_enemy_start(pos_start_of_turn.at(i)));
        input.push_back(move_enemy_start(pos_start_of_turn.at(i),dice_roll));
        input.push_back(is_home(pos_start_of_turn.at(i)));
    }
    switch(decision)
    {
    case 0:
        output.push_back(1);
        output.push_back(0);
        output.push_back(0);
        output.push_back(0);
        break;

    case 1:
        output.push_back(0);
        output.push_back(1);
        output.push_back(0);
        output.push_back(0);
        break;

    case 2:
        output.push_back(0);
        output.push_back(0);
        output.push_back(1);
        output.push_back(0);
        break;

    case 3:
        output.push_back(0);
        output.push_back(0);
        output.push_back(0);
        output.push_back(1);
        break;
    }
    emit select_piece(decision);
}

void ludo_player_ann::post_game_analysis(std::vector<int> relative_pos){
    pos_end_of_turn = relative_pos;
    bool game_complete = true;
    for(int i = 0; i < 4; ++i){
        if(pos_end_of_turn[i] < 99){
            game_complete = false;
        }
    }
    emit turn_complete(game_complete);
}

bool ludo_player_ann::move_out_home(int pos, int dice)
{
    if(pos == -1 && dice == 6)
    {
        return true;
    }
    return false;
}
bool ludo_player_ann::is_globe(int pos)
{
    if(pos == 8 ||pos == 21 ||pos == 34 ||pos == 47)
        return true;
    return false;
}

bool ludo_player_ann::is_global_safe(int token_index, int dice)
{
    if( pos_start_of_turn[token_index] < 0)
        return 0;

    if(is_globe(pos_start_of_turn.at(token_index)+dice))
        return true;

    if(pos_start_of_turn.at(token_index) == 0)
        return true;

    for(int i = 0; i < 4; i++)
    {
        if(i != token_index)
        {
           if(pos_start_of_turn.at(token_index)+dice == pos_start_of_turn.at(i) && pos_start_of_turn[i] != 99)
               return true;
        }
    }
    return false;
}

bool ludo_player_ann::move_global_safe(int token_index, int dice)
{
   if(is_global_safe(token_index,dice))
       return true;
   return false;
}

bool ludo_player_ann::is_star(int pos)
{
    if(pos == 5 || pos == 11 || pos == 18 || pos == 24 || pos == 31 || pos == 37 || pos == 44 || pos == 50)
        return true;
    return false;
}

int ludo_player_ann::star_jump(int pos,int dice)
{
    int jump = 0;
    if((pos + dice) == 5 || (pos + dice) == 18 || (pos + dice) == 31 || (pos + dice) == 44 ){
        jump = 6;
    }else if((pos + dice) == 11 || (pos + dice) == 24 || (pos + dice) == 37){
        jump=  7;
    }
    return (pos + dice + jump);
}

bool ludo_player_ann::move_star(int pos, int dice)
{
    if(is_star(pos+dice))
    {
        int next_star = star_jump(pos, dice);
        int occupancy = 0;
        for (int i = 0;i<pos_start_of_turn.size();i++)
        {
            if (pos_start_of_turn.at(i) == next_star)
                occupancy++;
        }
        if(occupancy < 2)
            return true;
    }
    return false;
}

bool ludo_player_ann::is_local_safe(int pos)
{
    bool on_star = is_star(pos);
    for(int i = 4; i < pos_start_of_turn.size(); i++)
    {
        if(pos - pos_start_of_turn.at(i) > 6)
            return true;
        if(on_star)
        {
            int jump = 0;
            if((pos) == 5 || (pos) == 18 || (pos) == 31 || (pos) == 44 ){
                jump = 6;
            }else if((pos) == 11 || (pos) == 24 || (pos) == 37){
                jump=  7;
            }
            if(pos - pos_start_of_turn.at(i) - jump > 6)
            {
                return true;
            }
        }
    }
    return false;
}

bool ludo_player_ann::move_local_safe(int pos, int dice)
{
    if(is_local_safe(pos+dice))
        return true;

    return false;
}

bool ludo_player_ann::is_on_home_stretch(int pos){
    if(pos > 50 && pos != 99){
        return true;
    }else{
        return false;
    }
}

bool ludo_player_ann::can_move_to_home_stretch(int pos, int dice_roll){
    if(is_on_home_stretch(pos+dice_roll) && pos < 51){
        return true;
    }
    return false;
}


bool ludo_player_ann::is_enemy_start(int pos){
    if(pos == 13 || pos == 26 || pos == 39){
        return true;
    }
    return false;
}

bool ludo_player_ann::is_killable(int pos){
    if(is_globe(pos)){//Is on globe
        return false;
    }
    if(is_on_home_stretch(pos)){//Is inhomestretch
        return false;
    }
    int samePos = 0;
    for(int i = 4; i < pos_start_of_turn.size() ; i++){ //check if two players are on the same spot
       for(int j = 4; j < pos_start_of_turn.size(); j++){
           if(pos_start_of_turn[i] == pos_start_of_turn[j] && i != j && pos_start_of_turn[j] != -1 && pos_start_of_turn[i] != -1){
               samePos++;
           }
       }
       if(samePos != 0){ //If there were multiple tokens on one spot
           return false;
       }
    }
    if(is_enemy_start(pos))//Check if it is a enemy player start
        return false;

    return true;
}

bool ludo_player_ann::can_kill_enemy(int dice, int token_index){
    int pos = pos_start_of_turn.at(token_index);
    if (pos == -1 || pos == 99 || is_on_home_stretch(pos) && pos+dice > 50)
        return false;

    int jump = 0;

    for(int i = 4; i < pos_start_of_turn.size(); i++){ //Normal Kill
        if((pos_start_of_turn[token_index] + dice) == pos_start_of_turn[i]){
            if(is_killable(pos_start_of_turn[i]))
                return true;
        }
        if(is_star(pos_start_of_turn[token_index] + dice)){ //Star kill
            if((pos + dice) == 5 || (pos + dice) == 18 || (pos + dice) == 31 || (pos + dice) == 44 ){
                jump = 6;
            }
            if((pos + dice) == 11 || (pos + dice) == 24 || (pos + dice) == 37){
                jump=  7;
            }

            if((pos_start_of_turn[token_index] + dice + jump) == pos_start_of_turn[i]){
                return true;
            }
        }
    }
    return false;
}

bool ludo_player_ann::can_complete(int pos, int dice){
    if((pos + dice) == 57 || (pos + dice == 50)){
        return true;
    }

    return false;
}

bool ludo_player_ann::is_complete(int pos){
    if(pos == 99){
        return true;
    }
    return false;
}

void ludo_player_ann::print_inputs()
{
    for(int i = 0; i < 1; i++)
    {
        std::cout << "Sjerne? " << is_star(pos_start_of_turn.at(i)) << std::endl;
        std::cout << " Globus? " << is_globe(pos_start_of_turn.at(i)) << std::endl;
        std::cout << " Kan komme ud? " << move_out_home(pos_start_of_turn.at(i), dice_roll) << std::endl;
        std::cout << " lokalt sikker? " << is_local_safe(pos_start_of_turn.at(i)) << std::endl;
        std::cout << " bliv lokalt sikker " << move_local_safe(pos_start_of_turn.at(i), dice_roll) << std::endl;
        std::cout << " slå fjende ihjel? " << can_kill_enemy(dice_roll, i) << std::endl;
        std::cout << " globalt sikker? " << is_global_safe(i) << std::endl;
        std::cout << " bliv globalt sikker " << move_global_safe(i, dice_roll) << std::endl;
        std::cout << " til stjerne? " << move_star(pos_start_of_turn.at(i), dice_roll) << std::endl;
    }

}

std::vector<int> ludo_player_ann::get_ivector()
{
    return input;
}

std::vector<int> ludo_player_ann::get_ovector()
{
    return output;
}

bool ludo_player_ann::move_enemy_start(int pos, int dice){
    if(is_enemy_start(pos+dice)){
        return true;
    }
    return false;
}

bool ludo_player_ann::is_home(int pos){
    if(pos == -1)
        return true;
    return false;
}

void ludo_player_ann::clear_vector()
{
    input.clear();
    output.clear();
}

int ludo_player_ann::make_aggressive_decision(){

    if(dice_roll == 6){//Prioritize to move one out
        for(int i = 0; i < 4; ++i){
            if(pos_start_of_turn[i]<0){
                return i;
            }
        }
    }
    for(int i = 0; i < 4; ++i){ //If not able to move one out, then try to beat someone
        if(can_kill_enemy(dice_roll,i)){
            return i;
        }
    }
    int closestMove = 1000;
    int indexClosest = 0;
    for(int i = 0; i < 4; ++i){

        for(int j = 4; j < pos_start_of_turn.size();++j){
            if((abs((pos_start_of_turn[i]+dice_roll)-pos_start_of_turn[j])) < closestMove  && ((pos_start_of_turn[i]+dice_roll)-pos_start_of_turn[j]) < 0){
                closestMove = abs((pos_start_of_turn[i]+dice_roll)-pos_start_of_turn[j]);
                indexClosest = i;
            }
        }
    }
    if(pos_start_of_turn[indexClosest]>=0 && pos_start_of_turn[indexClosest] != 99){
        return indexClosest;
    }

    for(int i = 0; i < 4; ++i){
        if(pos_start_of_turn[i]>=0 && pos_start_of_turn[i] != 99){
            return i;
        }
    }
    for(int i = 0; i < 4; ++i){ //maybe they are all locked in
        if(pos_start_of_turn[i]<0){
            return i;
        }
    }
    return -1;

}
