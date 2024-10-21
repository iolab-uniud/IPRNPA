#include "IPRNPA_Helpers.hh"

using namespace EasyLocal::Debug;

int main(int argc, const char* argv[])
{
  ParameterBox main_parameters("main", "Main Program options");
  Parameter<string> instance("instance", "Input instance", main_parameters); 
  Parameter<unsigned> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);   
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);
  Parameter<double> swap_rate("swap_rate", "Probability of selecting a change_room move in BSA", main_parameters);
  Parameter<double> change_room_rate("change_room_rate", "Probability of selecting a change_room move", main_parameters);
  Parameter<double> swap_rooms_rate("swap_rooms_rate", "Probability of selecting a swap_rooms move", main_parameters);
  Parameter<double> swap_nurses_rate("swap_nurses_rate", "Probability of selecting a swap_nurses move", main_parameters);
  Parameter<double> swap_rooms_rate_2s("swap_rooms_rate_2s", "Probability of selecting a swap_rooms move in 2 stage SA", main_parameters);
  Parameter<double> swap_nurses_rate_2s("swap_nurses_rate_2s", "Probability of selecting a swap_nurses move in 2 stage SA", main_parameters);

  //default values
  swap_rate=0.25;
  change_room_rate=0; 
  swap_nurses_rate=0.001;
  swap_rooms_rate=0.001;
  swap_nurses_rate_2s=0;
  swap_rooms_rate_2s=0;

  CommandLineParameters::Parse(argc, argv, false, true);  
  if (!instance.IsSet())
    {
      cout << "Error: --main::instance filename option must always be set" << endl;
      return 1;
    }
  IPRNPA_Input in(instance);
  IPRNPA_Output init(in);  
  if (seed.IsSet())
    Random::SetSeed(seed);
  int soft_weight=100;
  IPRNPA_Transfer cc1 (in, 11*soft_weight, false);
  IPRNPA_AgeGroup cc2 (in, 1*soft_weight, false);
  IPRNPA_Gender cc3 (in, 5*soft_weight, false);
  IPRNPA_Equipment cc4 (in, 5*soft_weight, false);
  IPRNPA_Continuity cc5 (in, 1*soft_weight, false);
  IPRNPA_Skill cc6 (in, 5*soft_weight, false);
  IPRNPA_Excess cc7 (in,5*soft_weight,false);
  IPRNPA_TotalBalance cc8 (in,5*soft_weight,false);
  IPRNPA_ShiftBalance cc9 (in,5*soft_weight,false);
  IPRNPA_NursesperRoom cc10 (in,2*soft_weight, false);
  IPRNPA_WalkingCircle cc11 (in,0.05*soft_weight,false);
  IPRNPA_WalkingStar cc12 (in,0.05*soft_weight,false);

  vector<string> cost_names = {"Transfer","AgeGroup","Gender","Equipment","Continuity","Skill","ExcessWorkload","TotalBalance","ShiftBalance","NursesperRoom","WalkingCircle","WalkingStar"};

  IPRNPA_ChangeNurseDeltaContinuity cn_cc5(in, cc5);
  IPRNPA_ChangeNurseDeltaSkill cn_cc6(in, cc6);
  IPRNPA_ChangeNurseDeltaExcess cn_cc7(in, cc7);
  IPRNPA_ChangeNurseDeltaTotalBalance cn_cc8(in, cc8);
  IPRNPA_ChangeNurseDeltaShiftBalance cn_cc9(in, cc9);
  IPRNPA_ChangeNurseDeltaNursesperRoom cn_cc10(in, cc10);
  IPRNPA_ChangeNurseDeltaWalkingCircle cn_cc11(in, cc11);
  IPRNPA_ChangeNurseDeltaWalkingStar cn_cc12(in, cc12);

  IPRNPA_ChangeRoomDeltaTransfer cr_cc1(in, cc1);
  IPRNPA_ChangeRoomDeltaAgeGroup cr_cc2(in, cc2);
  IPRNPA_ChangeRoomDeltaGender cr_cc3(in, cc3);
  IPRNPA_ChangeRoomDeltaEquipment cr_cc4(in, cc4);
  IPRNPA_ChangeRoomDeltaNursesperRoom cr_cc10(in, cc10);
  IPRNPA_ChangeRoomDeltaWalkingCircle cr_cc11(in, cc11);
  IPRNPA_ChangeRoomDeltaWalkingStar cr_cc12(in, cc12);

  IPRNPA_SwapNursesDeltaContinuity sn_cc5(in, cc5);
  IPRNPA_SwapNursesDeltaSkill sn_cc6(in, cc6);
  IPRNPA_SwapNursesDeltaExcess sn_cc7(in,cc7);
  IPRNPA_SwapNursesDeltaTotalBalance sn_cc8(in,cc8);
  IPRNPA_SwapNursesDeltaShiftBalance sn_cc9(in,cc9);
  IPRNPA_SwapNursesDeltaNursesperRoom sn_cc10(in, cc10);
  IPRNPA_SwapNursesDeltaWalkingCircle sn_cc11(in, cc11);
  IPRNPA_SwapNursesDeltaWalkingStar sn_cc12(in, cc12);

  IPRNPA_SwapRoomsDeltaTransfer      sr_cc1(in, cc1);
  IPRNPA_SwapRoomsDeltaAgeGroup      sr_cc2(in, cc2);
  IPRNPA_SwapRoomsDeltaGender        sr_cc3(in, cc3);
  IPRNPA_SwapRoomsDeltaEquipment     sr_cc4(in, cc4);
  IPRNPA_SwapRoomsDeltaNursesperRoom sr_cc10(in, cc10);
  IPRNPA_SwapRoomsDeltaWalkingCircle sr_cc11(in, cc11);
  IPRNPA_SwapRoomsDeltaWalkingStar   sr_cc12(in, cc12);

  IPRNPA_SolutionManager sm(in);

  IPRNPA_SolutionManager ts_sm(in);
  IPRNPA_ChangeNurseNeighborhoodExplorer cnhe(in, sm);
  IPRNPA_ChangeRoomNeighborhoodExplorer crhe(in, sm);
  IPRNPA_SwapNursesNeighborhoodExplorer snhe(in, sm);
  IPRNPA_SwapRoomsNeighborhoodExplorer srhe(in, sm);

  IPRNPA_ChangeRoomNeighborhoodExplorer rcrhe(in, ts_sm);
  IPRNPA_SwapRoomsNeighborhoodExplorer rsrhe(in, ts_sm);

  sm.AddCostComponent(cc1);
  sm.AddCostComponent(cc2);
  sm.AddCostComponent(cc3);
  sm.AddCostComponent(cc4);
  sm.AddCostComponent(cc5);
  sm.AddCostComponent(cc6);
  sm.AddCostComponent(cc7);
  sm.AddCostComponent(cc8);
  sm.AddCostComponent(cc9);
  sm.AddCostComponent(cc10);
  sm.AddCostComponent(cc11);
  sm.AddCostComponent(cc12);

  ts_sm.AddCostComponent(cc1);
  ts_sm.AddCostComponent(cc2);
  ts_sm.AddCostComponent(cc3);
  ts_sm.AddCostComponent(cc4);

  cnhe.AddDeltaCostComponent(cn_cc5);
  cnhe.AddDeltaCostComponent(cn_cc6);
  cnhe.AddDeltaCostComponent(cn_cc7);
  cnhe.AddDeltaCostComponent(cn_cc8);
  cnhe.AddDeltaCostComponent(cn_cc9);
  cnhe.AddDeltaCostComponent(cn_cc10);
  cnhe.AddDeltaCostComponent(cn_cc11);
  cnhe.AddDeltaCostComponent(cn_cc12);

  crhe.AddDeltaCostComponent(cr_cc1);
  crhe.AddDeltaCostComponent(cr_cc2);
  crhe.AddDeltaCostComponent(cr_cc3);
  crhe.AddDeltaCostComponent(cr_cc4);
  crhe.AddDeltaCostComponent(cr_cc10);
  crhe.AddDeltaCostComponent(cr_cc11);
  crhe.AddDeltaCostComponent(cr_cc12);

  rcrhe.AddDeltaCostComponent(cr_cc1);
  rcrhe.AddDeltaCostComponent(cr_cc2);
  rcrhe.AddDeltaCostComponent(cr_cc3);
  rcrhe.AddDeltaCostComponent(cr_cc4);

  snhe.AddDeltaCostComponent(sn_cc5);
  snhe.AddDeltaCostComponent(sn_cc6);
  snhe.AddDeltaCostComponent(sn_cc7);
  snhe.AddDeltaCostComponent(sn_cc8);
  snhe.AddDeltaCostComponent(sn_cc9);
  snhe.AddDeltaCostComponent(sn_cc10);
  snhe.AddDeltaCostComponent(sn_cc11);
  snhe.AddDeltaCostComponent(sn_cc12);

  srhe.AddDeltaCostComponent(sr_cc1);
  srhe.AddDeltaCostComponent(sr_cc2);
  srhe.AddDeltaCostComponent(sr_cc3);
  srhe.AddDeltaCostComponent(sr_cc4);
  srhe.AddDeltaCostComponent(sr_cc10);
  srhe.AddDeltaCostComponent(sr_cc11);
  srhe.AddDeltaCostComponent(sr_cc12);

  rsrhe.AddDeltaCostComponent(sr_cc1);
  rsrhe.AddDeltaCostComponent(sr_cc2);
  rsrhe.AddDeltaCostComponent(sr_cc3);
  rsrhe.AddDeltaCostComponent(sr_cc4);

  SetUnionNeighborhoodExplorer<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>, decltype(cnhe), decltype(crhe)> 
  bnhe(in, sm, "Bimodal Change Nurse/Room", cnhe, crhe, {1.0 - change_room_rate, change_room_rate});
  SetUnionNeighborhoodExplorer<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>, decltype(cnhe), decltype(crhe),decltype(snhe)> 
  tr1he(in, sm, "Trimodal Change Nurse/Room", cnhe, crhe, snhe,{1.0 - (change_room_rate+swap_nurses_rate),change_room_rate,swap_nurses_rate});
  SetUnionNeighborhoodExplorer<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>, decltype(cnhe), decltype(crhe),decltype(srhe)> 
  tr2he(in, sm, "Trimodal Change Nurse/Room", cnhe, crhe,srhe,{1.0 - (change_room_rate+swap_rooms_rate),change_room_rate,swap_rooms_rate});
  SetUnionNeighborhoodExplorer<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>, decltype(cnhe), decltype(crhe),decltype(srhe),decltype(snhe)> 
  qnhe(in, sm, "Quadmodal Change Nurse/Room", cnhe, crhe, srhe, snhe, {1.0 - (change_room_rate+swap_nurses_rate+swap_rooms_rate),change_room_rate,swap_rooms_rate,swap_nurses_rate});
  
  SetUnionNeighborhoodExplorer<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>, decltype(rcrhe), decltype(rsrhe)> 
  rms_bnhe(in, ts_sm, "Bimodal Change/Swap Room", rcrhe, rsrhe,{1.0 - swap_rooms_rate_2s, swap_rooms_rate_2s});
  SetUnionNeighborhoodExplorer<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>, decltype(cnhe), decltype(snhe)> 
  ts_bnhe(in, ts_sm, "Bimodal Change/Swap Nurse", cnhe, snhe,{1.0 - swap_nurses_rate_2s, swap_nurses_rate_2s});

  HillClimbing<IPRNPA_Input, IPRNPA_Output, IPRNPA_ChangeNurse, DefaultCostStructure<double>> chc(in, sm, cnhe, "CHC");
  SteepestDescent<IPRNPA_Input, IPRNPA_Output, IPRNPA_ChangeNurse, DefaultCostStructure<double>> csd(in, sm, cnhe, "CSD");
  SimulatedAnnealing<IPRNPA_Input, IPRNPA_Output, IPRNPA_ChangeNurse, DefaultCostStructure<double>> csa(in, sm, cnhe, "CSA");
  SimulatedAnnealing<IPRNPA_Input, IPRNPA_Output,decltype(bnhe)::MoveType,DefaultCostStructure<double>> bsa(in, sm, bnhe, "IPRNPA_BSA");
  SimulatedAnnealing<IPRNPA_Input, IPRNPA_Output,decltype(qnhe)::MoveType,DefaultCostStructure<double>> qsa(in, sm, qnhe, "IPRNPA_SA");
  SimulatedAnnealing<IPRNPA_Input, IPRNPA_Output,decltype(tr1he)::MoveType,DefaultCostStructure<double>> tr1_sa(in, sm, tr1he, "IPRNPA_TR1SA");
  SimulatedAnnealing<IPRNPA_Input, IPRNPA_Output,decltype(tr2he)::MoveType,DefaultCostStructure<double>> tr2_sa(in, sm, tr2he, "IPRNPA_TR2SA");
  SimulatedAnnealing<IPRNPA_Input, IPRNPA_Output,decltype(rms_bnhe)::MoveType,DefaultCostStructure<double>> ts_bsa(in, ts_sm, rms_bnhe, "IPRNPA_1sBSA");
  SimulatedAnnealing<IPRNPA_Input, IPRNPA_Output,decltype(ts_bnhe)::MoveType,DefaultCostStructure<double>> ts2_bsa(in, ts_sm, ts_bnhe, "IPRNPA_2sBSA");

  Tester<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>> tester(in, sm);
  MoveTester<IPRNPA_Input, IPRNPA_Output, IPRNPA_ChangeNurse, DefaultCostStructure<double>> change_nurse_move_test(in, sm, cnhe, "IPRNPA_ChangeNurse move", tester);
  change_nurse_move_test.SetTolerance(0.01);
  MoveTester<IPRNPA_Input, IPRNPA_Output, IPRNPA_ChangeRoom, DefaultCostStructure<double>> change_room_move_test(in, sm, crhe, "IPRNPA_ChangeRoom move", tester);
  change_room_move_test.SetTolerance(0.01);
  MoveTester<IPRNPA_Input, IPRNPA_Output, IPRNPA_SwapNurses, DefaultCostStructure<double>> swap_nurses_test(in, sm, snhe, "IPRNPA_SwapNurses move", tester);
  swap_nurses_test.SetTolerance(0.01);
  MoveTester<IPRNPA_Input, IPRNPA_Output, IPRNPA_SwapRooms, DefaultCostStructure<double>> swap_rooms_test(in, sm, srhe, "IPRNPA_SwapRooms move", tester);
  swap_rooms_test.SetTolerance(0.01);
  MoveTester<IPRNPA_Input, IPRNPA_Output,decltype(bnhe)::MoveType,DefaultCostStructure<double>> bimodal_test(in, sm, bnhe, "IPRNPA_Bimodal move", tester); 
  bimodal_test.SetTolerance(0.01);
  MoveTester<IPRNPA_Input, IPRNPA_Output,decltype(qnhe)::MoveType,DefaultCostStructure<double>> quadmodal_test(in, sm, qnhe, "IPRNPA_Quadmodal move", tester); 
  quadmodal_test.SetTolerance(0.01);

  SimpleLocalSearch<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>> solver(in, sm, "IPRNPA solver");
  SimpleLocalSearch<IPRNPA_Input, IPRNPA_Output, DefaultCostStructure<double>> twosolver(in, ts_sm, "IPRNPA 2 stage solver");
  if (!CommandLineParameters::Parse(argc, argv, true, false))
    return 1;

  if (!method.IsSet())
    { 
      if (init_state.IsSet())
        tester.RunMainMenu(init_state);
      else
        tester.RunMainMenu();
    }
  else
    {
      if (method == string("CSA"))
        {
          solver.SetRunner(csa);
        }
      else if (method == string("CHC"))
        {
          solver.SetRunner(chc);
        }
      else if (method == string("CSD"))
        {
          solver.SetRunner(csd);
        }
      else if (method == string("BSA"))
        {
          solver.SetRunner(bsa);
        }
      else if (method == string("TS_BSA"))
        {
          twosolver.SetRunner(ts_bsa);
        }
      else if (method == string("TR1SA"))
        {
          solver.SetRunner(tr1_sa);
        }
      else if (method == string("TR2SA"))
        {
          solver.SetRunner(tr2_sa);
        }
      else if (method == string("SA"))
        {
          solver.SetRunner(qsa);
        }
      else
        {
          cerr << "Unknown method " << static_cast<string>(method) << endl;
        }

    if(method == string("TS_BSA")){
        auto result = twosolver.Solve();
        IPRNPA_Output out = result.output;
        IPRNPA_ChangeNurseNeighborhoodExplorer ts_cnhe(in, ts_sm);
        IPRNPA_SwapNursesNeighborhoodExplorer ts_snhe(in, ts_sm);

        ts_sm.AddCostComponent(cc5);
        ts_sm.AddCostComponent(cc6);
        ts_sm.AddCostComponent(cc7);
        ts_sm.AddCostComponent(cc8);
        ts_sm.AddCostComponent(cc9);
        ts_sm.AddCostComponent(cc10);
        ts_sm.AddCostComponent(cc11);
        ts_sm.AddCostComponent(cc12);
        
        ts_cnhe.AddDeltaCostComponent(cn_cc5);
        ts_cnhe.AddDeltaCostComponent(cn_cc6);
        ts_cnhe.AddDeltaCostComponent(cn_cc7);
        ts_cnhe.AddDeltaCostComponent(cn_cc8);
        ts_cnhe.AddDeltaCostComponent(cn_cc9);
        ts_cnhe.AddDeltaCostComponent(cn_cc10);
        ts_cnhe.AddDeltaCostComponent(cn_cc11);
        ts_cnhe.AddDeltaCostComponent(cn_cc12);

        ts_snhe.AddDeltaCostComponent(sn_cc5);
        ts_snhe.AddDeltaCostComponent(sn_cc6);
        ts_snhe.AddDeltaCostComponent(sn_cc7);
        ts_snhe.AddDeltaCostComponent(sn_cc8);
        ts_snhe.AddDeltaCostComponent(sn_cc9);
        ts_snhe.AddDeltaCostComponent(sn_cc10);
        ts_snhe.AddDeltaCostComponent(sn_cc11);
        ts_snhe.AddDeltaCostComponent(sn_cc12);
        twosolver.SetRunner(ts2_bsa);
        result = twosolver.Resolve(out);
        out = result.output;

        if (output_file.IsSet())
        { 
          double cc_cost=0;
          ofstream os(static_cast<string>(output_file).c_str());
          string info="Cost: "+ to_string(result.cost.total/soft_weight) + " ";
          for (size_t i = 0; i < result.cost.all_components.size(); i++){
                  info+=cost_names[i]+": "+to_string(result.cost.all_components[i]/soft_weight)+", ";
                  cc_cost+=result.cost.all_components[i];
          }
          info+="Sum of cost components"+to_string(cc_cost/soft_weight)+' ';
          info+= "Time: "+ to_string(result.running_time)+"s\n";
          out.PrettyPrint(os,info);
          os.close();
        }
        else
          { 
            //Info/j2r output
            cout << "{" 
               << "\"cost\": " <<  result.cost.total/soft_weight <<  ", "
               << "\"time\": " <<  result.running_time <<  ", ";
              for (size_t i = 0; i < result.cost.all_components.size(); i++)
                  cout<<"\""<<cost_names[i]<<" "<<"\": "<< result.cost.all_components[i]<<", ";
              cout<< "\"seed\": " << Random::GetSeed() << "} " << endl;      
          //Irace output
          //cout<<result.cost.total/soft_weight<<endl;
          }
      }
      else{
      auto result = solver.Solve();
      IPRNPA_Output out = result.output;
      if (output_file.IsSet())
        {
          double cc_cost=0;
          ofstream os(static_cast<string>(output_file).c_str());
          string info="Cost: "+ to_string(result.cost.total/soft_weight) + " ";
          for (size_t i = 0; i < result.cost.all_components.size(); i++){
                  info+=cost_names[i]+": "+to_string(result.cost.all_components[i]/soft_weight)+", ";
                  cc_cost+=result.cost.all_components[i];
          }
          info+="Sum of cost components"+to_string(cc_cost/soft_weight)+' ';
          info+= "Time: "+ to_string(result.running_time)+"s\n";
          out.PrettyPrint(os,info);
          os.close();
        }
      else
        {
          //Info/j2r output
          cout << "{" 
               << "\"cost\": " <<  result.cost.total/soft_weight <<  ", "
               << "\"time\": " <<  result.running_time <<  ", ";
              for (size_t i = 0; i < result.cost.all_components.size(); i++)
                  cout<<"\""<<cost_names[i]<<" "<<"\": "<< result.cost.all_components[i]<<", ";
              cout<< "\"seed\": " << Random::GetSeed() << "} " << endl;      
          //Irace output
          //cout<<result.cost.total/soft_weight<<endl;
        }
   }
  }
  return 0;
}
