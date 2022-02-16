//
// Created by Till Konrad Aust on 17.12.21.
//
// in case of simulation set flag
#define SIMULATION


#include "kilogrid_stub.h"

/*-----------------------------------------------------------------------------------------------*/
/* Initialization of the Loopfunctions.                                                          */
/*-----------------------------------------------------------------------------------------------*/
CKilogrid::CKilogrid():
CLoopFunctions() {}

CKilogrid::~CKilogrid() {}



/*-----------------------------------------------------------------------------------------------*/
/* Init method runs before every experiment starts.                                              */
/*-----------------------------------------------------------------------------------------------*/
void CKilogrid::Init(TConfigurationNode &t_tree) {
    // random tool for selecting messages and more
    m_pcRNG = CRandom::CreateRNG("argos");

    // get all Kilobots
    get_kilobots_entities();

    // read configuration - map and parameters
    read_configuration(t_tree);
    // run the setup message of the Kilogrid
    for(int x_it = 0; x_it < 10; x_it++){
        for(int y_it = 0; y_it < 20; y_it++){
            setup(x_it, y_it);
        }
    }

    // initialize some helpers to track the kilobots - only needed in sim
    robot_positions.resize(kilobot_entities.size());

    Reset();

}


/*-----------------------------------------------------------------------------------------------*/
/* Gets called when the experiment is resetted.                                                  */
/*-----------------------------------------------------------------------------------------------*/
void CKilogrid::Reset() {
  timeStep = 0;
    /*
  * When the 'reset' method is called on the kilobot controller, the
  * kilobot state is destroyed and recreated. Thus, we need to
  * recreate the list of controllers and debugging info from scratch
  * as well.
  */
  m_tKBs.clear();
  /* Get the map of all kilobots from the space */
  CSpace::TMapPerType& tKBMap = GetSpace().GetEntitiesByType("kilobot");
  /* Go through them */
  for(CSpace::TMapPerType::iterator it = tKBMap.begin();
    it != tKBMap.end();
    ++it) {
   /* Create a pointer to the current kilobot */
   CKilobotEntity* pcKB = any_cast<CKilobotEntity*>(it->second);
   CCI_KilobotController* pcKBC = &dynamic_cast<CCI_KilobotController&>(pcKB->GetControllableEntity().GetController());
   /* Create debug info for controller */
   debug_info_t* ptDebugInfo = pcKBC->DebugInfoCreate<debug_info_t>();
   /* Append to list */
   m_tKBs.push_back(std::make_pair(pcKBC, ptDebugInfo));
  }
}


/*-----------------------------------------------------------------------------------------------*/
/* Gets called when the experiment ended.                                                        */
/*-----------------------------------------------------------------------------------------------*/
void CKilogrid::Destroy() {
    // TODO implement what should happen if simulation quits

}


/*-----------------------------------------------------------------------------------------------*/
/* Gets called before every simulation step.                                                     */
/*-----------------------------------------------------------------------------------------------*/
void CKilogrid::PreStep(){
  timeStep++;
    // collect all the data from the robots - and virtualize them
    virtual_message_reception();
//NOT USED IN AGGREGATION
    // // simulate IR message reception for each cell
    // for(int x_it = 0; x_it < 10; x_it++){
    //     for(int y_it = 0; y_it < 20; y_it++){
    //         if(module_memory[x_it][y_it].robot_messages.size() > 0){
    //             // IMPORTANT: cellid, distance and crc error are not used in simulation: here they are
    //             // dummy values
    //             IR_rx(x_it, y_it, module_memory[x_it][y_it].robot_messages[m_pcRNG->Uniform(CRange<UInt32>(0,module_memory[x_it][y_it].robot_messages.size()))], cell_id[0], d, 0);
    //             module_memory[x_it][y_it].robot_messages.clear();
    //         }
    //     }
    // }
    //
    // // simulate reception of can messages
    // for(int x_it = 0; x_it < 10; x_it++){
    //     for(int y_it = 0; y_it < 20; y_it++){
    //         // if we have multiple messages at the same time select a random one
    //         if(module_memory[x_it][y_it].received_cell_messages.size() > 0){
    //             // shuffle and take first element (this way because we also consider case == 1)
    //             CAN_rx(x_it, y_it, &module_memory[x_it][y_it].received_cell_messages[m_pcRNG->Uniform(CRange<UInt32>(0,module_memory[x_it][y_it].received_cell_messages.size()))]);
    //             // clear list afterwards
    //             module_memory[x_it][y_it].received_cell_messages.clear();
    //         }
    //     }
    // }

    // run the loop
    if(timeStep%5 == 0) //kilogrid send messages every 0.5 seconds
    {
      for(int x_it = 0; x_it < 10; x_it++){
          for(int y_it = 0; y_it < 20; y_it++){
              loop(x_it, y_it);
          }
        }
    }
}


/*-----------------------------------------------------------------------------------------------*/
/* Gets called after every simulation step.                                                      */
/*-----------------------------------------------------------------------------------------------*/
void CKilogrid::PostStep(){
  /* Go through the kilobots */
 for(size_t i = 0; i < m_tKBs.size(); ++i) {
    /* Create a pointer to the kilobot state */
    kilobot_state_t* ptState = m_tKBs[i].first->GetRobotState();
    /* Print current state internal robot state */
    LOG << m_tKBs[i].first->GetId() << ": "                       << std::endl
        << "\tkilo_ticks_debug: "           << m_tKBs[i].second->kilo_ticks_debug << std::endl
        << "\ttimer_go_straight_debug: "           << m_tKBs[i].second->timer_go_straight_debug << std::endl
        << "\tpleave: "           << m_tKBs[i].second->pleave_debug << std::endl;
 }
}


/*-----------------------------------------------------------------------------------------------*/
/* This function reads the config file for the kilogrid and saves the content to configuration.  */
/*-----------------------------------------------------------------------------------------------*/
void CKilogrid::read_configuration(TConfigurationNode& t_node){
    // getting config script name from .argos file
    TConfigurationNode& tExperimentVariablesNode = GetNode(t_node,"variables");
    GetNodeAttribute(tExperimentVariablesNode, "config_file", config_file_name);

    // variables used for reading
    int tmp_x_module;
    int tmp_y_module;
    int tmp_value;
    int read_mode = 0; // 1 is address, 2 is data
    std::string tmp_str;

    // reading
    input.open(config_file_name);
    for( std::string line; getline( input, line ); ){
        // for each line
        if(!line.empty() && line[0] != '#'){  // exclude comments and empty lines

            //printf("%s\n",line.c_str()); // <- this way you can pring the line out!
            if (line == "address"){
                read_mode = 1;
            }else if(line == "data"){
                read_mode = 2;
            } else{
                // reading address
                if(read_mode == 1){
                    // isolating numbers
                    tmp_str = line;
                    tmp_str = tmp_str.substr(7);  // remove module
                    std::string::size_type p  = tmp_str.find('-');
                    tmp_y_module = std::stoi(tmp_str.substr(p+1));
                    tmp_x_module = std::stoi(tmp_str);
                }else if(read_mode == 2){ // read data
                    tmp_value = std::stoi(line, 0, 16);
                    configuration[tmp_x_module][tmp_y_module].push_back(tmp_value);
                }
            }
        }
    }
}


void CKilogrid::set_LED_with_brightness(int x, int y, cell_num_t cn, color_t color, brightness_t brightness){
    module_memory[x][y].cell_colour[cn] = color;
    GetSpace().GetFloorEntity().SetChanged();
}


/*-----------------------------------------------------------------------------------------------*/
/* Visualization of the floor color.                                                             */
/*-----------------------------------------------------------------------------------------------*/
CColor CKilogrid::GetFloorColor(const CVector2 &vec_position_on_plane) {
    int floor_color = position2option(vec_position_on_plane);

    // checks if the option is in time
    if(floor_color == 1){
        return CColor::RED;
    }else if(floor_color == 2){
        return CColor::BLUE;
    }else if(floor_color == 3){
        return CColor::WHITE;
    }else if(floor_color == 4){
        return CColor::BLACK;
    }

    return CColor::GREEN; //to visually debug
}


/*-----------------------------------------------------------------------------------------------*/
/* Returns the Option of the given position.                                                     */
/*-----------------------------------------------------------------------------------------------*/
UInt16 CKilogrid::position2option(CVector2 t_position){
    int module_x = t_position.GetX()*10;
    int module_y = t_position.GetY()*10;

    uint8_t cell = 0;
    int x = t_position.GetX()*20;
    int y = t_position.GetY()*20;

    if (x >= 20) x = 19;
    if (y >= 40) y = 39;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    // get the cell
    if(x % 2 == 0 && y % 2 == 1){
        cell = 0;
    }else if(x % 2 == 1 && y % 2 == 1){
        cell = 1;
    }else if(x % 2 == 0 && y % 2 == 0){
        cell = 2;
    }else if(x % 2 == 1 && y % 2 == 0){
        cell = 3;
    }

    return module_memory[module_x][module_y].cell_colour[cell];
}


CVector2 CKilogrid::position2cell(CVector2 t_position){
    int x = t_position.GetX()*20;
    int y = t_position.GetY()*20;

    if (x >= 20) x = 19;
    if (y >= 40) y = 39;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    return CVector2(x,y);
}


CVector2 CKilogrid::position2module(CVector2 t_position) {
    float x = t_position.GetX() * 10;
    float y = t_position.GetY() * 10;

    if (x >= 10) x = 9;
    if (y >= 20) y = 19;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    return CVector2(int(x),int(y));
}


CVector2 CKilogrid::module2cell(CVector2 module_pos, cell_num_t cn){
    int cur_x = module_pos.GetX() * 2;
    int cur_y = module_pos.GetY() * 2;

    switch(cn){
        case 0:
            cur_y += 1;
            break;
        case 1:
            cur_x += 1;
            cur_y += 1;
            break;
        case 2:
            break;
        case 3:
            cur_x += 1;
    }

    return CVector2(cur_x, cur_y);
}


void CKilogrid::get_kilobots_entities(){
    // Get the map of all kilobots from the space
    CSpace::TMapPerType& mapKilobots=GetSpace().GetEntitiesByType("kilobot");
    // Go through them
    for(CSpace::TMapPerType::iterator it = mapKilobots.begin();it != mapKilobots.end();++it) {
        kilobot_entities.push_back(any_cast<CKilobotEntity*>(it->second));
    }
}


/*-----------------------------------------------------------------------------------------------*/
/* Returns the position of the kilobot.                                                          */
/*-----------------------------------------------------------------------------------------------*/
CVector2 CKilogrid::GetKilobotPosition(CKilobotEntity& c_kilobot_entity){
    CVector2 vecKP(c_kilobot_entity.GetEmbodiedEntity().GetOriginAnchor().Position.GetX(),
                   c_kilobot_entity.GetEmbodiedEntity().GetOriginAnchor().Position.GetY());
    return vecKP;
}


/*-----------------------------------------------------------------------------------------------*/
/* Returns the id of the kilobot.                                                                */
/*-----------------------------------------------------------------------------------------------*/
UInt16 CKilogrid::GetKilobotId(CKilobotEntity& c_kilobot_entity){
    std::string strKilobotID((c_kilobot_entity).GetControllableEntity().GetController().GetId());
    return std::stoul(strKilobotID.substr(2));
}

void CKilogrid::virtual_message_reception(){
    // get position information, later used for sending stuff!!
    for(UInt16 it = 0; it < kilobot_entities.size(); it++) {
        robot_positions[GetKilobotId(*kilobot_entities[it])] = position2cell(
                GetKilobotPosition(*kilobot_entities[it]));
    }

//NOT USED IN AGGREGATION, ROBOTS DO NOT SEND ANYTHING TO THE KILOGRID (EXCEPT FOR TRACKING BUT THIS IS OMITTED IN SIM)
    // for(UInt16 it = 0; it < debug_info_kilobots.size(); it++) {
    //     if (debug_info_kilobots[it]->broadcast_flag == 1) {
    //         int module_x = int(robot_positions[GetKilobotId(*kilobot_entities[it])].GetX())/2;
    //         int module_y = int(robot_positions[GetKilobotId(*kilobot_entities[it])].GetY())/2;
    //         IR_message_t* tmp_msg = new IR_message_t;
    //         tmp_msg->type = debug_info_kilobots[it]->type;
    //         tmp_msg->data[0] = debug_info_kilobots[it]->data0;
    //         tmp_msg->data[1] = debug_info_kilobots[it]->data1;
    //         tmp_msg->data[2] = debug_info_kilobots[it]->data2;
    //         tmp_msg->data[3] = debug_info_kilobots[it]->data3;
    //         tmp_msg->data[4] = debug_info_kilobots[it]->data4;
    //         tmp_msg->data[5] = debug_info_kilobots[it]->data5;
    //         tmp_msg->data[6] = debug_info_kilobots[it]->data6;
    //         tmp_msg->data[7] = debug_info_kilobots[it]->data7;
    //         module_memory[module_x][module_y].robot_messages.push_back(tmp_msg);
    //     }
    // }
}


void CKilogrid::set_IR_message(int x, int y, IR_message_t &m, cell_num_t cn) {
    // get correct cell
    CVector2 cell_pos = module2cell(CVector2(x,y), cn);

    // get kilobots on this cell
    kilobot_entities_vector current_robots;

    for(uint8_t it=0;it < kilobot_entities.size();it++){
        if(robot_positions[GetKilobotId(*kilobot_entities[it])].GetX() == cell_pos.GetX()
        and robot_positions[GetKilobotId(*kilobot_entities[it])].GetY() == cell_pos.GetY()){
            current_robots.push_back(kilobot_entities[it]);
        }
    }

    // send to all kilobots on this cell
    for(UInt16 it=0; it < current_robots.size(); it++){
        message_t message_to_send;
        message_to_send.type = m.type;
        message_to_send.data[0] = m.data[0];
        message_to_send.data[1] = m.data[1];
        message_to_send.data[2] = m.data[2];
        message_to_send.data[3] = m.data[3];
        message_to_send.data[4] = m.data[4];
        message_to_send.data[5] = m.data[5];
        message_to_send.data[6] = m.data[6];  
        message_to_send.data[7] = m.data[7];
        GetSimulator().GetMedium<CKilobotCommunicationMedium>("kilocomm").SendOHCMessageTo(*current_robots[it], &message_to_send);
    }
}


void CKilogrid::init_CAN_message(CAN_message_t* cell_msg){
    uint8_t d;

    cell_msg->id = 0;
    cell_msg->header.rtr = 0;
    cell_msg->header.length = 8;

    for(d = 0; d < 8; d++) cell_msg->data[d] = 0;
}


uint8_t CKilogrid::CAN_message_tx(CAN_message_t *m, kilogrid_address_t add) {
    // this is a hack because we can only virtually set msgs
    // TODO is this message coppied or just the pointer
    // module_memory[add.x][add.y].received_cell_message = m;
    module_memory[add.x][add.y].received_cell_messages.push_back(*m);

    return 1;
}




/*-----------------------------------------------------------------------------------------------*/
/* The following methods need to be implemented by the user - they aim to be such as the         */
/* methods used on the real Kilogrid.                                                            */
/*-----------------------------------------------------------------------------------------------*/
void CKilogrid::setup(int x, int y){
    module_memory[x][y].cell_colour[0] = color_t (configuration[x][y][0]);
    module_memory[x][y].cell_colour[1] = color_t (configuration[x][y][1]);
    module_memory[x][y].cell_colour[2] = color_t (configuration[x][y][2]);
    module_memory[x][y].cell_colour[3] = color_t (configuration[x][y][3]);
}


void CKilogrid::loop(int x, int y){
  for(int i = 0; i < 4; ++i){
    switch(module_memory[x][y].cell_colour[i]){
        case 1:
            set_LED_with_brightness(x, y, cell_id[i], module_memory[x][y].cell_colour[i], HIGH);
            module_memory[x][y].ir_message_tx->type = 1;
            module_memory[x][y].ir_message_tx->data[0] = 22; //grey colour
            module_memory[x][y].ir_message_tx->data[1] = 1; //obstacle true
            set_IR_message(x, y, *module_memory[x][y].ir_message_tx, cell_id[i]);
            break;
        case 2:
            set_LED_with_brightness(x, y, cell_id[i], module_memory[x][y].cell_colour[i], HIGH);
            module_memory[x][y].ir_message_tx->type = 1;
            module_memory[x][y].ir_message_tx->data[0] = 22; //grey colour
            module_memory[x][y].ir_message_tx->data[1] = 0; //obstacle false
            set_IR_message(x, y, *module_memory[x][y].ir_message_tx, cell_id[i]);
            break;
        case 3:
            set_LED_with_brightness(x, y, cell_id[i], module_memory[x][y].cell_colour[i], HIGH);
            module_memory[x][y].ir_message_tx->type = 1;
            module_memory[x][y].ir_message_tx->data[0] = 21; //white colour
            module_memory[x][y].ir_message_tx->data[1] = 0; //obstacle false
            set_IR_message(x, y, *module_memory[x][y].ir_message_tx, cell_id[i]);
            break;
        case 4:
            set_LED_with_brightness(x, y, cell_id[i], module_memory[x][y].cell_colour[i], HIGH);
            module_memory[x][y].ir_message_tx->type = 1;
            module_memory[x][y].ir_message_tx->data[0] = 20; //black colour
            module_memory[x][y].ir_message_tx->data[1] = 0; //obstacle false
            set_IR_message(x, y, *module_memory[x][y].ir_message_tx, cell_id[i]);
            break;
        default:
            break;
       }
	}
}


void CKilogrid::IR_rx(int x, int y, IR_message_t *m, cell_num_t c, distance_measurement_t *d, uint8_t CRC_error) {
    // used for debug tracking on the real kilogrid ...
//    if(!CRC_error && m->type == TRACKING) {
//        CAN_message_t *msg = next_CAN_message();
//        if (msg != NULL) { // if the buffer is not full
//            serialize_tracking_message(msg, c, m->data);
//        }
//    }else if(!CRC_error && m->type == VIRTUAL_ROBOT_MSG){
    // check if msg is zero ... return no msg to send
//    if (m == nullptr){
//        return;
//    }
    if (!CRC_error && m->type == MSG_T_VIRTUAL_ROBOT_MSG) {
          // some example usage
//        module_memory[x][y].received_option = m->data[0];
//        module_memory[x][y].received_com_range = m->data[1];
//        module_memory[x][y].received_x = m->data[2];
//        module_memory[x][y].received_y = m->data[3];
        module_memory[x][y].msg_number_current = m->data[4];
        if (module_memory[x][y].msg_number_current != module_memory[x][y].msg_number) {
            // case new message
            module_memory[x][y].msg_number = module_memory[x][y].msg_number_current;
            // TODO implement logic ...
        } else {
            // message already seen -> discard
            return;
        }
    }
}

void CKilogrid::CAN_rx(int x, int y, CAN_message_t *m){
    // example usage of this method
    if (m->data[0] == 55){  // set msg
        module_memory[x][y].cell_received_op[0] = m->data[1];
        module_memory[x][y].cell_received_op[1] = m->data[2];
        module_memory[x][y].cell_received_op[2] = m->data[3];
        module_memory[x][y].cell_received_op[3] = m->data[4];
        // receive_timer = m->data[5];
        for(uint8_t cell_it_cb = 0; cell_it_cb < 4; cell_it_cb++){
            if(module_memory[x][y].cell_received_op[cell_it_cb] != 0){
                module_memory[x][y].reset_timer[cell_it_cb] = 0;
            }
        }
    }else {
        module_memory[x][y].cell_received_op[0] = 3;
        module_memory[x][y].cell_received_op[1] = 3;
        module_memory[x][y].cell_received_op[2] = 3;
        module_memory[x][y].cell_received_op[3] = 3;
    }
}


REGISTER_LOOP_FUNCTIONS(CKilogrid, "kilogrid_loop_functions")
