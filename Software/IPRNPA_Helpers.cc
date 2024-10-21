#include "IPRNPA_Helpers.hh"
#include<cmath>


IPRNPA_SolutionManager::IPRNPA_SolutionManager(const IPRNPA_Input & pin) 
  : SolutionManager<IPRNPA_Input,IPRNPA_Output,DefaultCostStructure<cost_type>>(pin, "IPRNPASolutionManager")  {} 

void IPRNPA_SolutionManager::DumpState(const IPRNPA_Output& out, ostream& os) const
{
  out.Dump(os);
}

void IPRNPA_SolutionManager::RandomState(IPRNPA_Output& out) 
{
  unsigned p, i, ss, es, sd, ed;
  int r, n, old_r;
  out.Reset();
  for(p=0;p<in.Patients();p++){
    ss=in.GetPatient(p).admission-1; 
    es=in.GetPatient(p).discharge; 
    sd=ss/3; 
    ed=es/3; 
    r = Random::Uniform<int>(0,in.Rooms()-1);
    old_r=-1;
    for(i=sd;i<ed;i++){
      while(out.Occupancy(r,i)>=in.GetRoom(r).capacity){
        r = Random::Uniform<int>(0,in.Rooms()-1); 
      }
      out.SetRoom(p,i,r);
      out.AddPatient(p,i,r); 
      if(old_r!=-1 && old_r!=r)
        out.AddTransfer(p);
      old_r=r; 
      out.UpdateRoomData(p,i);
    }
    for(i=ss;i<es;i++){
      do{
          n = Random::Uniform<int>(0,in.Nurses()-1);
      }while(!in.NurseWorking(n,i));
      out.SetShift(p,i,n);
      out.UpdateNurseData(p,i,n);
    }  
  }
  out.ComputeTotalLoad();
}

bool IPRNPA_SolutionManager::CheckConsistency(const IPRNPA_Output& out) const
{
  for(unsigned r=0;r<in.Rooms();r++){
    for(unsigned d=0;d<in.Days();d++){
      if(out.Occupancy(r,d)>in.GetRoom(r).capacity)
        return false;
    }
  }
  return true;
}

void IPRNPA_SolutionManager::PrettyPrintOutput(const IPRNPA_Output& out, string filename) const
{
  ofstream os(filename);
  out.PrettyPrint(os,"");
  if (!CheckConsistency(out))
  {
    cerr << "State non consistent" << endl;
    exit(1);
  }
  os.close();
}

cost_type IPRNPA_Transfer::ComputeCost(const IPRNPA_Output& out) const
{
  int cost = 0;
  for(unsigned p=0;p<in.Patients();p++){
    for(unsigned d=1;d<in.Days();d++){
      if(out.RoomAssignment(p,d-1)!=-1 && out.RoomAssignment(p,d)!=-1 && out.RoomAssignment(p,d-1)!=out.RoomAssignment(p,d))
        cost++;
    }
  }
  return cost;
}

void IPRNPA_Transfer::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  for(unsigned p=0;p<in.Patients();p++){
    for(unsigned d=1;d<in.Days();d++){
      if(out.RoomAssignment(p,d-1)!=-1 && out.RoomAssignment(p,d)!=-1 && out.RoomAssignment(p,d-1)!=out.RoomAssignment(p,d))
        os<<"Patient "<<p+1<<" moves from room "<<out.RoomAssignment(p,d-1)+1<<" to room "<<out.RoomAssignment(p,d)+1<<" between days "<<d<<"-"<<d+1<<endl;
    }
  }
}

cost_type IPRNPA_AgeGroup::ComputeCost(const IPRNPA_Output& out) const
{ 
  int cost = 0;
  unsigned age_max,age_min,curr_age;
  for(unsigned r=0;r<in.Rooms();r++){
    for(unsigned d=0;d<in.Days();d++){
      if(out.Occupancy(r,d)>1){
        bool first=true;
        for(unsigned s=0;s<out.Occupancy(r,d);s++){
          curr_age=in.GetPatient(out.GetOccupant(r,d,s)).agegroup;
          if(first){
            age_max=curr_age;
            age_min=curr_age;
            first=false;
          }
          else if(in.GetPatient(out.GetOccupant(r,d,s)).agegroup>age_max)
            age_max=curr_age;
          else if(in.GetPatient(out.GetOccupant(r,d,s)).agegroup<age_min)
            age_min=curr_age;
        }
      cost+=age_max-age_min;
      }
    }
  }
  return cost;
}

void IPRNPA_AgeGroup::PrintViolations(const IPRNPA_Output& out, ostream& os) const
{
  unsigned age_max,age_min,curr_age;
  for(unsigned r=0;r<in.Rooms();r++){
    for(unsigned d=0;d<in.Days();d++){
      bool first=true;
      for(unsigned s=0;s<out.Occupancy(r,d);s++){
        curr_age=in.GetPatient(out.GetOccupant(r,d,s)).agegroup;
        if(first){
          age_max=curr_age;
          age_min=curr_age;
          first=false;
        }
        else if(in.GetPatient(out.GetOccupant(r,d,s)).agegroup>age_max)
          age_max=curr_age;
        else if(in.GetPatient(out.GetOccupant(r,d,s)).agegroup<age_min)
          age_min=curr_age;
      }
    if(age_max-age_min>0)
      os<<"Age difference in room "<<r+1<<" on day "<<d+1<<" is of "<<age_max-age_min<<endl;
    }
  }
}

cost_type IPRNPA_Gender::ComputeCost(const IPRNPA_Output& out) const
{ 
  int cost = 0;
  for(unsigned r=0;r<in.Rooms();r++){
    for(unsigned d=0;d<in.Days();d++){
      for(unsigned s=1;s<out.Occupancy(r,d);s++){
        if(in.GetPatient(out.GetOccupant(r,d,s-1)).gender!=in.GetPatient(out.GetOccupant(r,d,s)).gender){
          cost++;
          break;
        }
      }
    }
  }
  return cost;
}

void IPRNPA_Gender::PrintViolations(const IPRNPA_Output& out, ostream& os) const
{
    for(unsigned r=0;r<in.Rooms();r++){
      for(unsigned d=0;d<in.Days();d++){
        for(unsigned s=1;s<out.Occupancy(r,d);s++){
          if(in.GetPatient(out.GetOccupant(r,d,s-1)).gender!=in.GetPatient(out.GetOccupant(r,d,s)).gender){
            os<<"Room "<<r+1<<" is mixed on day "<<d+1<<"(e.g. patient: "<<out.GetOccupant(r,d,s-1)+1<<" and patient "<<out.GetOccupant(r,d,s)+1<<")"<<endl;
            break;
          }
        }
      }
  }
}

cost_type IPRNPA_Equipment::ComputeCost(const IPRNPA_Output& out) const
{ 
  int cost = 0,is;
  Room current_room;
  bool found;
  for(unsigned p=0;p<in.Patients();p++){
    for(unsigned s=in.GetPatient(p).admission-1;s<in.GetPatient(p).discharge;s=s+3){
      if(in.GetPatient(p).equip_req.size()!=0){
        current_room=in.GetRoom(out.RoomAssignment(p,s/3));
        is=s-(in.GetPatient(p).admission-1);
        for(unsigned e=0;e<in.GetPatient(p).equip_req[is].size();e++){
          found=false;
          for(unsigned i=0;i<current_room.equipments.size();i++){
            if(current_room.equipments[i]==in.GetPatient(p).equip_req[is][e]){
              found=true;
              break;
            }   
          }
          if(!found){
            cost++;
            break;
          }
        }
      }
    }
  }
  return cost;
}

void IPRNPA_Equipment::PrintViolations(const IPRNPA_Output& out, ostream& os) const
{
  Room current_room;
  bool found;
  int is;
  for(unsigned p=0;p<in.Patients();p++){
    for(unsigned s=in.GetPatient(p).admission-1;s<in.GetPatient(p).discharge;s=s+3){
      if(in.GetPatient(p).equip_req.size()!=0){
        current_room=in.GetRoom(out.RoomAssignment(p,s/3));
        is=s-(in.GetPatient(p).admission-1);
        for(unsigned e=0;e<in.GetPatient(p).equip_req[is].size();e++){
          found=false;
          for(unsigned i=0;i<current_room.equipments.size();i++){
            if(current_room.equipments[i]==in.GetPatient(p).equip_req[is][e]){
              found=true;
              break;
            }   
          }
          if(!found){
            os<<"Equipment "<<in.FindEquipmentString(in.GetPatient(p).equip_req[is][e])<<" requested by patient "<<p+1<<" is not available in room "<<out.RoomAssignment(p,s/3)+1<<" during shift "<<s+1<<endl;
            break;
          }
        }
      }
    }
  }
}

cost_type IPRNPA_Continuity::ComputeCost(const IPRNPA_Output& out) const
{ 
  int cost = 0;
  vector<bool> nurses_assigned;
  for(unsigned p=0;p<in.Patients();p++){
    nurses_assigned.resize(in.Nurses(),false);
    for(unsigned s=in.GetPatient(p).admission-1;s<in.GetPatient(p).discharge;s++){
      if(!nurses_assigned[out.NurseAssignment(p,s)]){
        nurses_assigned[out.NurseAssignment(p,s)]=true;
        cost++;
      }
    }
    nurses_assigned.clear();
  }
  return cost;
}

void IPRNPA_Continuity::PrintViolations(const IPRNPA_Output& out, ostream& os) const
{
  vector<bool> nurses_assigned;
  int old_cost=0,cost=0;
  for(unsigned p=0;p<in.Patients();p++){
    nurses_assigned.resize(in.Nurses(),false);
    old_cost=cost;
    for(unsigned s=in.GetPatient(p).admission-1;s<in.GetPatient(p).discharge;s++){
      if(!nurses_assigned[out.NurseAssignment(p,s)]){
        nurses_assigned[out.NurseAssignment(p,s)]=true;
        cost++;
      }
    }
    nurses_assigned.clear();
    os<<"Patient "<<p+1<<" is assigned to "<<cost-old_cost<<" different nurses "<<endl;
  }
}

cost_type IPRNPA_Skill::ComputeCost(const IPRNPA_Output& out) const
{ 
  int cost = 0,ss;
  for(unsigned p=0;p<in.Patients();p++){
    ss=in.GetPatient(p).admission-1;
    for(unsigned s=ss;s<in.GetPatient(p).discharge;s++){
      if(((s%3)!=2) && in.GetNurse(out.NurseAssignment(p,s)).skill_lev<in.GetPatient(p).skill_req[s-ss])
        cost++;
    }
  }
  return cost;
}

void IPRNPA_Skill::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  int ss;
  for(unsigned p=0;p<in.Patients();p++){
    ss=in.GetPatient(p).admission-1;
    for(unsigned s=ss;s<in.GetPatient(p).discharge;s++){
      if(((s%3)!=2) && in.GetNurse(out.NurseAssignment(p,s)).skill_lev<in.GetPatient(p).skill_req[s-ss])
        os<<"Nurse "<<out.NurseAssignment(p,s)<<" does not have skill level required to cover shift "<<s+1<<" for patient "<<p+1<<endl;
    }
  }
}

cost_type IPRNPA_Excess::ComputeCost(const IPRNPA_Output& out) const
{ 
  cost_type cost = 0;
  int ss,ws;
  vector<float> load(in.Nurses(),0);
  for(unsigned i=0;i<in.Shifts();i++){
    for(unsigned p=0;p<in.Patients();p++){
      ss=in.GetPatient(p).admission-1;
      if(out.NurseAssignment(p,i)!=-1){
        load[out.NurseAssignment(p,i)]+=in.GetPatient(p).workload_req[i-ss];
      }
    }
    for(unsigned n=0;n<in.Nurses();n++){
      ws=in.SearchWs(n,i);
      if(ws!=-1 && load[n]>in.GetNurse(n).max_loads[ws])
        cost+=(load[n]-in.GetNurse(n).max_loads[ws]);
    }
    fill(load.begin(),load.end(),0);
  }
  return cost;
}

void IPRNPA_Excess::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  int ss,ws;
  vector<double> load(in.Nurses(),0);
  for(unsigned i=0;i<in.Shifts();i++){
    for(unsigned p=0;p<in.Patients();p++){
      ss=in.GetPatient(p).admission-1;
      if(out.NurseAssignment(p,i)!=-1){
        load[out.NurseAssignment(p,i)]+=in.GetPatient(p).workload_req[i-ss];
      }
    }
    for(unsigned n=0;n<in.Nurses();n++){
      ws=in.SearchWs(n,i);
      if(ws!=-1 && load[n]>in.GetNurse(n).max_loads[ws])
        os<<"Nurse "<<n<<" works load:"<<load[n]<<" during shift "<<i+1<<" more then the load requested:"<<in.GetNurse(n).max_loads[ws]<<"(cost="<<load[n]-in.GetNurse(n).max_loads[ws]<<")"<<endl;
    }
    fill(load.begin(),load.end(),0);
  }
}

cost_type IPRNPA_TotalBalance::ComputeCost(const IPRNPA_Output& out) const
{ 
  cost_type cost = 0, ss,ns;
  vector<float> load(in.Nurses(),0);
  vector<double> total_load(in.Nurses(),0);
  for(unsigned s=0;s<in.Shifts();s++){
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        ss=in.GetPatient(p).admission-1;
        load[out.NurseAssignment(p,s)]+=in.GetPatient(p).workload_req[s-ss];
      }
    }
    for(unsigned n=0;n<in.Nurses();n++){
      if(in.NurseWorking(n,s)){
        ns=in.SearchWs(n,s);
        total_load[n]+=(load[n]/in.GetNurse(n).max_loads[ns]);
      }
    }
    std::fill(load.begin(),load.end(),0);
  }
  for(unsigned n1=0;n1<in.Nurses()-1;n1++){
    for(unsigned n2=n1+1;n2<in.Nurses();n2++)
      cost+=abs(total_load[n1]-total_load[n2]);
  }
  return cost;
}

void IPRNPA_TotalBalance::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  int ss, ns;
  vector<float> load(in.Nurses(),0);
  vector<double> total_load(in.Nurses(),0);
  for(unsigned s=0;s<in.Shifts();s++){
    for(unsigned p=0;p<in.Patients();p++){
      ss=in.GetPatient(p).admission-1;
      if(out.NurseAssignment(p,s)!=-1)
        load[out.NurseAssignment(p,s)]+=in.GetPatient(p).workload_req[s-ss];
    }
    for(unsigned n=0;n<in.Nurses();n++){
      if(in.NurseWorking(n,s)){
        ns=in.SearchWs(n,s);
        total_load[n]+=(load[n]/static_cast<double>(in.GetNurse(n).max_loads[ns]));
      }
    }
    std::fill(load.begin(),load.end(),0);
  }
  for(unsigned n1=0;n1<in.Nurses()-1;n1++){
    for(unsigned n2=n1+1;n2<in.Nurses();n2++){
      if(abs(total_load[n1]-total_load[n2])!=0)
        os<<"Total load for nurse "<<n1<<" is "<<total_load[n1]<<" while for nurse "<< n2<<" is "<<total_load[n2]<<"(cost="<<abs(total_load[n1]-total_load[n2])<<")"<<endl;
    }
  }
}

cost_type IPRNPA_ShiftBalance::ComputeCost(const IPRNPA_Output& out) const
{ 
  cost_type cost = 0, ss, ws1,ws2;
  vector<float> load(in.Nurses(),0);
  for(unsigned s=0;s<in.Shifts();s++){
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        ss=in.GetPatient(p).admission-1;
        load[out.NurseAssignment(p,s)]+=in.GetPatient(p).workload_req[s-ss];
      }
    }
    float n1_per,n2_per;
    for(unsigned n1=0;n1<in.Nurses()-1;n1++){
      if(in.NurseWorking(n1,s)){
        ws1=in.SearchWs(n1,s);
        n1_per=(load[n1]/in.GetNurse(n1).max_loads[ws1]);
        for(unsigned n2=n1+1;n2<in.Nurses();n2++){
          if(in.NurseWorking(n2,s)){
            ws2=in.SearchWs(n2,s);
            n2_per=load[n2]/in.GetNurse(n2).max_loads[ws2];
            cost+=abs(n1_per-n2_per);
          }
        }
      }
    }
    fill(load.begin(),load.end(),0);
  }
  return cost;
}

void IPRNPA_ShiftBalance::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  int ss, ws1,ws2;
  vector<float> load(in.Nurses(),0);
  for(unsigned s=0;s<in.Shifts();s++){
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        ss=in.GetPatient(p).admission-1;
        load[out.NurseAssignment(p,s)]+=in.GetPatient(p).workload_req[s-ss];
      }
    }
    float n1_per,n2_per;
    for(unsigned n1=0;n1<in.Nurses()-1;n1++){
      if(in.NurseWorking(n1,s)){
        ws1=in.SearchWs(n1,s);
        n1_per=(load[n1]/in.GetNurse(n1).max_loads[ws1]);
        for(unsigned n2=n1+1;n2<in.Nurses();n2++){
          if(in.NurseWorking(n2,s)){
            ws2=in.SearchWs(n2,s);
            n2_per=load[n2]/in.GetNurse(n2).max_loads[ws2];
            if(abs(n1_per-n2_per)!=0)
              os<<"Nurse "<<n1<<" works "<<n1_per<<" wrt his/her optimal load while "<<n2<<" works "<<n2_per<<" wrt his/her optimal during shift "<<s+1<<"(cost="<<abs(n1_per-n2_per)<<")"<<endl;
          }
        }
      }
    }
    fill(load.begin(),load.end(),0);
  } 
}

cost_type IPRNPA_NursesperRoom::ComputeCost(const IPRNPA_Output& out) const
{ 
  int cost = 0,nurse,room;
  vector<unsigned> nurses_in_room(in.Rooms(),0);
  for(unsigned s=0;s<in.Shifts();s++){
    vector<vector<bool>> in_room(in.Rooms(),vector<bool> (in.Nurses(),false));
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        nurse=out.NurseAssignment(p,s);
        room=out.RoomAssignment(p,s/3);
        if(!in_room[room][nurse]){
          cost++;
          nurses_in_room[room]++;
          in_room[room][nurse]=true;
        }
      }
    }
    fill(nurses_in_room.begin(),nurses_in_room.end(),0);
  }
  return cost;
}

void IPRNPA_NursesperRoom::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  int nurse,room;
  vector<unsigned> nurses_in_room(in.Rooms(),0);
  for(unsigned s=0;s<in.Shifts();s++){
    vector<vector<bool>> in_room(in.Rooms(),vector<bool> (in.Nurses(),false));
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        nurse=out.NurseAssignment(p,s);
        room=out.RoomAssignment(p,s/3);
        if(!in_room[room][nurse]){
          nurses_in_room[room]++;
          in_room[room][nurse]=true;
        }
      }
    }
    for(unsigned r=0;r<in.Rooms();r++){
      if(nurses_in_room[r]>0)
        os<<"During shift "<<s+1<<" "<<nurses_in_room[r]<<" nurses work in room "<< r+1<<endl;
    }
    fill(nurses_in_room.begin(),nurses_in_room.end(),0);
  }
}

cost_type IPRNPA_WalkingCircle::ComputeCost(const IPRNPA_Output& out) const
{ 
  cost_type cost=0;
  int nurse,room;
  double cost_shift;
  vector<vector<bool>> nurse_in_room;
  for(unsigned s=0;s<in.Shifts();s++){
    nurse_in_room.resize(in.Nurses(),vector<bool>(in.Rooms(),false));
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        nurse=out.NurseAssignment(p,s);
        room=out.RoomAssignment(p,s/3);
        if(!nurse_in_room[nurse][room])
          nurse_in_room[nurse][room]=true;
      }
    }
    cost_shift=0;
    for(unsigned i=0;i<in.Nurses();i++){
      if(in.NurseWorking(i,s)){
        for(unsigned j=0;j<in.Rooms();j++){
          if(nurse_in_room[i][j]){
            for(unsigned k=j+1;k<in.Rooms();k++){
              if(j!=k && nurse_in_room[i][k])
                cost_shift+=((in.GetDistance(j+1,k+1)+in.GetDistance(k+1,j+1))*(in.GetShift(s).c_weight))/2;
            }
          }
        }
      }
    }
    cost+=cost_shift;
    nurse_in_room.clear();
  }
  return cost;
}

void IPRNPA_WalkingCircle::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  int nurse,room;
  float cost_shift;
  vector<vector<bool>> nurse_in_room;
  for(unsigned s=0;s<in.Shifts();s++){
    nurse_in_room.resize(in.Nurses(),vector<bool>(in.Rooms(),false));
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        nurse=out.NurseAssignment(p,s);
        room=out.RoomAssignment(p,s/3);
        if(!nurse_in_room[nurse][room])
          nurse_in_room[nurse][room]=true;
      }
    }
    cost_shift=0;
    for(unsigned i=0;i<in.Nurses();i++){
      if(in.NurseWorking(i,s)){
        for(unsigned j=0;j<in.Rooms();j++){
          if(nurse_in_room[i][j]){
            for(unsigned k=0;k<in.Rooms();k++){
              if(j!=k && nurse_in_room[i][k]){
                cost_shift+=in.GetDistance(j+1,k+1);
              }
            }
          }
        }
      }
    }
    os<<"Cost of walking circle distances for shift  "<<s+1<<":"<<cost_shift*(in.GetShift(s).c_weight)*0.5<<endl;
    nurse_in_room.clear();
  }
}

cost_type IPRNPA_WalkingStar::ComputeCost(const IPRNPA_Output& out) const
{ 
  cost_type cost=0,nurse,room;
  vector<vector<bool>> nurse_in_room;
  for(unsigned s=0;s<in.Shifts();s++){
    nurse_in_room.resize(in.Nurses(),vector<bool>(in.Rooms(),false));
    float s_weight=in.GetShift(s).s_weight;
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        nurse=out.NurseAssignment(p,s);
        room=out.RoomAssignment(p,s/3);
        if(!nurse_in_room[nurse][room])
          nurse_in_room[nurse][room]=true;
      }
    }
    for(unsigned i=0;i<in.Nurses();i++){
      if(in.NurseWorking(i,s)){
        for(unsigned j=0;j<in.Rooms();j++){
          if(nurse_in_room[i][j]){
            cost+=in.GetDistance(0,j+1)*s_weight;
            }
        }
      }
    }
    nurse_in_room.clear();
  }
  return cost;
}

void IPRNPA_WalkingStar::PrintViolations(const IPRNPA_Output& out, ostream& os) const{
  int nurse,room;
  float cost_shift;
  vector<vector<bool>> nurse_in_room;
  for(unsigned s=0;s<in.Shifts();s++){
    nurse_in_room.resize(in.Nurses(),vector<bool>(in.Rooms(),false));
    for(unsigned p=0;p<in.Patients();p++){
      if(out.NurseAssignment(p,s)!=-1){
        nurse=out.NurseAssignment(p,s);
        room=out.RoomAssignment(p,s/3);
        if(!nurse_in_room[nurse][room])
          nurse_in_room[nurse][room]=true;
      }
    }
    cost_shift=0;
    for(unsigned i=0;i<in.Nurses();i++){
      if(in.NurseWorking(i,s)){
        for(unsigned j=0;j<in.Rooms();j++){
          if(nurse_in_room[i][j]){
            cost_shift+=in.GetDistance(0,j+1);
            }
        }
      }
    }
    os<<"Cost of walking star distances for shift "<<s+1<<":"<<cost_shift*(in.GetShift(s).s_weight)<<endl;
    nurse_in_room.clear();
  }
}
/*****************************************************************************
 * IPRNPA_ChangeNurse Neighborhood Explorer Methods
 *****************************************************************************/
// ------------- IPRNPA_ChangeNurse move
bool operator==(const IPRNPA_ChangeNurse& mv1, const IPRNPA_ChangeNurse& mv2)
{
  return (mv1.patient==mv2.patient &&
    mv1.shift==mv2.shift && 
    mv1.new_nurse==mv2.new_nurse);
}

bool operator!=(const IPRNPA_ChangeNurse& mv1, const IPRNPA_ChangeNurse& mv2)
{
  return (mv1.patient!=mv2.patient ||
    mv1.shift!=mv2.shift || 
    mv1.new_nurse!=mv2.new_nurse);
}

bool operator<(const IPRNPA_ChangeNurse& mv1, const IPRNPA_ChangeNurse& mv2)
{
  return (mv1.patient < mv2.patient)
    || (mv1.patient == mv2.patient && mv1.shift < mv2.shift)
    || (mv1.patient == mv2.patient && mv1.shift == mv2.shift && mv1.new_nurse < mv2.new_nurse);
}

istream& operator>>(istream& is, IPRNPA_ChangeNurse& mv)
{
  char ch;
  is>>mv.shift>>ch>>mv.patient>>ch>>mv.old_nurse>>ch>>mv.new_nurse;
	return is;
}

ostream& operator<<(ostream& os, const IPRNPA_ChangeNurse& mv)
{
  os<<mv.patient<<"("<<mv.shift<<","<<mv.old_nurse<<"<->"<<mv.new_nurse<<")"<<endl;
  return os;
}

void IPRNPA_ChangeNurseNeighborhoodExplorer::RandomMove(const IPRNPA_Output& st, IPRNPA_ChangeNurse& mv) const
{
  int index_1,index_2,count=0;
  do
    {
      mv.old_nurse = Random::Uniform<int>(0,in.Nurses()-1);
      Nurse nd=in.GetNurse(mv.old_nurse);
      index_1=Random::Uniform<int>(0,nd.working_shifts.size()-1);
      mv.shift=nd.working_shifts[index_1];
      count++;
    }
  while (st.NumPatients(mv.old_nurse,index_1) == 0);
  index_2=Random::Uniform<int>(0,st.NumPatients(mv.old_nurse,index_1)-1);
  mv.patient=st.ExtractPatient(mv.old_nurse,index_1,index_2);
  do{
    mv.new_nurse=Random::Uniform<int>(0,in.Nurses()-1);
    count++;
  } while(count<100 && (mv.old_nurse==mv.new_nurse || !in.NurseWorking(mv.new_nurse,mv.shift)));
  if(count>=100)
    throw EmptyNeighborhood();
} 

bool IPRNPA_ChangeNurseNeighborhoodExplorer::FeasibleMove(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const
{
  return  mv.new_nurse!=mv.old_nurse;
} 

void IPRNPA_ChangeNurseNeighborhoodExplorer::MakeMove(IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const
{ 
  out.ChangeNurse(mv.patient,mv.shift,mv.new_nurse);
}  

void IPRNPA_ChangeNurseNeighborhoodExplorer::FirstMove(const IPRNPA_Output& out, IPRNPA_ChangeNurse& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
    AnyNextMove(out,mv);	
}

void IPRNPA_ChangeNurseNeighborhoodExplorer::AnyFirstMove(const IPRNPA_Output& out, IPRNPA_ChangeNurse& mv) const
{
  mv.patient=0;
  mv.shift=0;
  while(out.RoomAssignment(mv.patient,mv.shift/3)==-1){
    mv.shift+=3;
  }
  mv.old_nurse=out.NurseAssignment(mv.patient,mv.shift);
  mv.new_nurse=0;
  while(!in.NurseWorking(mv.new_nurse,mv.shift)){
    mv.new_nurse++;
  }
}

bool IPRNPA_ChangeNurseNeighborhoodExplorer::NextMove(const IPRNPA_Output& out, IPRNPA_ChangeNurse& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IPRNPA_ChangeNurseNeighborhoodExplorer::AnyNextMove(const IPRNPA_Output& out, IPRNPA_ChangeNurse& mv) const
{
  if (NextNurse(out,mv)) 
    {
      return true;
    }
  else if (NextShift(out,mv)) 
    {
      FirstNurse(out,mv); 
      return true;
    }
  else if (mv.patient < in.Patients() - 1) 
    {
      mv.patient++;
      mv.shift=in.GetPatient(mv.patient).admission-1;
      FirstNurse(out,mv);
      return true;
    }
  else return false;
}

bool IPRNPA_ChangeNurseNeighborhoodExplorer::NextNurse(const IPRNPA_Output& out, IPRNPA_ChangeNurse& mv) const{
  do{
    mv.new_nurse++;
  }while(mv.new_nurse<in.Nurses() && (!in.NurseWorking(mv.new_nurse,mv.shift)));
  return mv.new_nurse<in.Nurses();
}

void IPRNPA_ChangeNurseNeighborhoodExplorer::FirstNurse(const IPRNPA_Output& out, IPRNPA_ChangeNurse& mv) const{
  mv.old_nurse=out.NurseAssignment(mv.patient,mv.shift);
  mv.new_nurse=-1;
  do{
    mv.new_nurse++;
  } while(!in.NurseWorking(mv.new_nurse,mv.shift));
}

bool IPRNPA_ChangeNurseNeighborhoodExplorer::NextShift(const IPRNPA_Output& out, IPRNPA_ChangeNurse& mv) const{
  mv.shift++;
  return mv.shift<in.Shifts() && out.RoomAssignment(mv.patient,mv.shift/3)!=-1;
}

/*****************************************************************************
 * IPRNPA_ChangeRoom Neighborhood Explorer Methods
 *****************************************************************************/
// ------------- IPRNPA_ChangeRoom move

bool operator==(const IPRNPA_ChangeRoom& mv1, const IPRNPA_ChangeRoom& mv2)
{
  return (mv1.patient==mv2.patient &&
    mv1.day==mv2.day && 
    mv1.new_room==mv2.new_room);
}

bool operator!=(const IPRNPA_ChangeRoom& mv1, const IPRNPA_ChangeRoom& mv2)
{
  return (mv1.patient!=mv2.patient ||
    mv1.day!=mv2.day || 
    mv1.new_room!=mv2.new_room);
}

bool operator<(const IPRNPA_ChangeRoom& mv1, const IPRNPA_ChangeRoom& mv2)
{
  return (mv1.patient < mv2.patient)
    || (mv1.patient == mv2.patient && mv1.day < mv2.day)
    || (mv1.patient == mv2.patient && mv1.day == mv2.day && mv1.new_room < mv2.new_room);
}

istream& operator>>(istream& is, IPRNPA_ChangeRoom& mv)
{
  char ch;
  is>>mv.patient>>ch>>mv.day>>ch>>mv.old_room>>ch>>mv.new_room;
	return is;
}

ostream& operator<<(ostream& os, const IPRNPA_ChangeRoom& mv)
{
  os<<mv.patient<<"("<<mv.day<<","<<mv.old_room<<"<->"<<mv.new_room<<")"<<endl;
  return os;
}

void IPRNPA_ChangeRoomNeighborhoodExplorer::RandomMove(const IPRNPA_Output& out, IPRNPA_ChangeRoom& mv) const
{
  int admission,discharge,capacity,occupancy,count=0;
  mv.patient = Random::Uniform<int>(0,in.Patients()-1);
  admission=(in.GetPatient(mv.patient).admission-1)/3;
  discharge=(in.GetPatient(mv.patient).discharge-1)/3;
  mv.day=Random::Uniform<int>(admission,discharge);
  mv.old_room=out.RoomAssignment(mv.patient,mv.day);
  do{
    mv.new_room=Random::Uniform<int>(0,in.Rooms()-1);
    capacity=in.GetRoom(mv.new_room).capacity;
    occupancy=out.Occupancy(mv.new_room,mv.day);
    count++;
  } while(count<=100 && (mv.old_room==mv.new_room || capacity<=occupancy)); 
  if(count>=100)
    throw EmptyNeighborhood();
} 

bool IPRNPA_ChangeRoomNeighborhoodExplorer::FeasibleMove(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const
{
  return true;
} 

void IPRNPA_ChangeRoomNeighborhoodExplorer::MakeMove(IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const
{ 
  out.ChangeRoom(mv.patient,mv.day,mv.new_room);
}  

void IPRNPA_ChangeRoomNeighborhoodExplorer::FirstMove(const IPRNPA_Output& out, IPRNPA_ChangeRoom& mv) const
{
  mv.patient=0;
  mv.day=(in.GetPatient(mv.patient).admission-1)/3;
  mv.old_room=out.RoomAssignment(mv.patient,mv.day);
  mv.new_room=0;
  int capacity=in.GetRoom(mv.new_room).capacity;
  while(mv.new_room!=mv.old_room && out.Occupancy(mv.new_room,mv.day)>=capacity){
    mv.new_room++;
  }
}

bool IPRNPA_ChangeRoomNeighborhoodExplorer::NextMove(const IPRNPA_Output& out, IPRNPA_ChangeRoom& mv) const
{
   if (NextRoom(out,mv)) 
    {
      return true;
    }
  else if (NextDay(out,mv)) 
    {
      FirstRoom(out,mv); 
      return true;
    }
  else if (mv.patient < in.Patients() - 1) 
    {
      mv.patient++;
      mv.day=(in.GetPatient(mv.patient).admission-1)/3;
      FirstRoom(out,mv);
      return true;
    }
  else return false;
}

bool IPRNPA_ChangeRoomNeighborhoodExplorer::NextRoom(const IPRNPA_Output& out, IPRNPA_ChangeRoom& mv) const{

  do{
    mv.new_room++;
  }while(mv.new_room<in.Rooms() && (mv.old_room==mv.new_room || (in.GetRoom(mv.new_room).capacity<=out.Occupancy(mv.new_room,mv.day))));
  return mv.new_room<in.Rooms();
}

void IPRNPA_ChangeRoomNeighborhoodExplorer::FirstRoom(const IPRNPA_Output& out, IPRNPA_ChangeRoom& mv) const{
  mv.old_room=out.RoomAssignment(mv.patient,mv.day);
  mv.new_room=-1;
  do{
    mv.new_room++;
  } while(mv.old_room==mv.new_room || (in.GetRoom(mv.new_room).capacity<=out.Occupancy(mv.new_room,mv.day))); 
}

bool IPRNPA_ChangeRoomNeighborhoodExplorer::NextDay(const IPRNPA_Output& out, IPRNPA_ChangeRoom& mv) const{
  mv.day++;
  if(mv.day<=(in.GetPatient(mv.patient).discharge-1)/3)
    return true;
  else return false;
}

// ------------- IPRNPA_SwapNurses move
bool operator==(const IPRNPA_SwapNurses& mv1, const IPRNPA_SwapNurses& mv2)
{
  return (mv1.shift==mv2.shift &&
    mv1.p1==mv2.p1 && 
    mv1.p2==mv2.p2);
}

bool operator!=(const IPRNPA_SwapNurses& mv1, const IPRNPA_SwapNurses& mv2)
{
  return (mv1.shift!=mv2.shift ||
    mv1.p1!=mv2.p1 || 
    mv1.p2!=mv2.p2);
}

bool operator<(const IPRNPA_SwapNurses& mv1, const IPRNPA_SwapNurses& mv2)
{
  return (mv1.shift < mv2.shift)
    || (mv1.shift == mv2.shift && mv1.p1 < mv2.p1)
    || (mv1.shift == mv2.shift && mv1.p1 == mv2.p1 && mv1.p2 < mv2.p2);
}

istream& operator>>(istream& is, IPRNPA_SwapNurses& mv)
{
  char ch;
  is>>mv.shift>>ch>>mv.p1>>ch>>mv.p2>>ch>>mv.n1>>ch>>mv.n2;
	return is;
}

ostream& operator<<(ostream& os, const IPRNPA_SwapNurses& mv)
{
  os<<mv.shift<<"("<<mv.shift/3<<")"<<"("<<mv.n1<<"("<<mv.p1<<")"<<"<->"<<mv.n2<<"("<<mv.p2<<")"<<")"<<endl;
  return os;
}

/**
 @remarks we assume there are at least 2 nurses covering a shift AND at least two patients covered by two different nurses p1<p2 to break symmetry
 if not I might loop (might happen on real_world instances)
*/
void IPRNPA_SwapNursesNeighborhoodExplorer::RandomMove(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const 
{
    int patient_1,patient_2,presences,count=0;
    do{
      do{
      mv.shift = Random::Uniform<int>(0,in.Shifts()-1);
      presences=in.PatientsPres(mv.shift);
      count++;
      }while (count<100 && presences<2);
      if(count>=100)
        throw EmptyNeighborhood();
      else
        count=0;
      patient_1=in.PatientInd(mv.shift,Random::Uniform<int>(0,presences-1));
      do{
        patient_2=in.PatientInd(mv.shift,Random::Uniform<int>(0,presences-1));
        }
      while(patient_1==patient_2);
      count=0;
      if(patient_1<patient_2){
        mv.p1=patient_1;
        mv.p2=patient_2;
      }
      else{
        mv.p1=patient_2;
        mv.p2=patient_1;
      }
      mv.n1=out.NurseAssignment(mv.p1,mv.shift);
      mv.n2=out.NurseAssignment(mv.p2,mv.shift); 
    }
    while(count<=100 && FeasibleMove(out,mv));
    if(count>=100)
        throw EmptyNeighborhood();
} 

bool IPRNPA_SwapNursesNeighborhoodExplorer::FeasibleMove(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const
{
  return mv.n1!=mv.n2;
} 

void IPRNPA_SwapNursesNeighborhoodExplorer::MakeMove(IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const
{ 
  out.SwapNurses(mv.shift,mv.p1,mv.p2);
}  

void IPRNPA_SwapNursesNeighborhoodExplorer::FirstMove(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
    AnyNextMove(out,mv);	
}

void IPRNPA_SwapNursesNeighborhoodExplorer::AnyFirstMove(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const
{
  mv.shift=-1;
  do{
    mv.shift++;
  }while(in.PatientsPres(mv.shift)<2);
  mv.p1=in.PatientInd(mv.shift,0);
  mv.p2=in.PatientInd(mv.shift,1);
  mv.n1=out.NurseAssignment(mv.p1,mv.shift);
  mv.n2=out.NurseAssignment(mv.p2,mv.shift);
  mv.p1_idx=0;
  mv.p2_idx=1;
}

bool IPRNPA_SwapNursesNeighborhoodExplorer::NextMove(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IPRNPA_SwapNursesNeighborhoodExplorer::AnyNextMove(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const
{
  if (NextPatientTwo(out,mv)) 
    {
      mv.p2_idx++;
      mv.p2=in.PatientInd(mv.shift,mv.p2_idx);
      mv.n2=out.NurseAssignment(mv.p2,mv.shift);
      return true;
    }
  else if (NextPatientOne(out,mv)) 
    {
      mv.p1_idx++;
      mv.p2_idx=mv.p1_idx+1;
      mv.p1=in.PatientInd(mv.shift,mv.p1_idx);
      mv.p2=in.PatientInd(mv.shift,mv.p2_idx);
      mv.n1=out.NurseAssignment(mv.p1,mv.shift);
      mv.n2=out.NurseAssignment(mv.p2,mv.shift);
      return true;
    }
  else if (NextShift(out,mv)) 
    {
      mv.p1=in.PatientInd(mv.shift,0);
      mv.p2=in.PatientInd(mv.shift,1);
      mv.p1_idx=0;
      mv.p2_idx=1;
      mv.n1=out.NurseAssignment(mv.p1,mv.shift);
      mv.n2=out.NurseAssignment(mv.p2,mv.shift);
      return true;
    }
  else return false;
}

bool IPRNPA_SwapNursesNeighborhoodExplorer::NextPatientOne(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const{
  int presences=in.PatientsPres(mv.shift);
  if(mv.p1_idx==presences-2)
    return false;
  return true;
}

bool IPRNPA_SwapNursesNeighborhoodExplorer::NextPatientTwo(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const{
  int presences=in.PatientsPres(mv.shift);
  if(mv.p2_idx==presences-1)
    return false;
  return true;
}

bool IPRNPA_SwapNursesNeighborhoodExplorer::NextShift(const IPRNPA_Output& out, IPRNPA_SwapNurses& mv) const{
  do{
    mv.shift++;
  }while(in.PatientsPres(mv.shift)<2);
  return mv.shift<in.Shifts();
}

// ------------- IPRNPA_SwapRooms move

bool operator==(const IPRNPA_SwapRooms& mv1, const IPRNPA_SwapRooms& mv2)
{
  return (mv1.day==mv2.day &&
    mv1.p1==mv2.p1 && 
    mv1.p2==mv2.p2);
}

bool operator!=(const IPRNPA_SwapRooms& mv1, const IPRNPA_SwapRooms& mv2)
{
  return (mv1.day!=mv2.day &&
    mv1.p1!=mv2.p1 && 
    mv1.p2!=mv2.p2);
}

bool operator<(const IPRNPA_SwapRooms& mv1, const IPRNPA_SwapRooms& mv2)
{
  return (mv1.day < mv2.day)
    || (mv1.day == mv2.day && mv1.p1 < mv2.p1)
    || (mv1.day == mv2.day && mv1.p1 == mv2.p1 && mv1.p2 < mv2.p2);
}

istream& operator>>(istream& is, IPRNPA_SwapRooms& mv)
{
  char ch;
  is>>mv.day>>ch>>mv.p1>>ch>>mv.p2;
	return is;
}

ostream& operator<<(ostream& os, const IPRNPA_SwapRooms& mv) //eventually add rooms
{
  os<<mv.day<<"("<<mv.p1<<"<->"<<mv.p2<<")"<<endl;
  return os;
}

void IPRNPA_SwapRoomsNeighborhoodExplorer::RandomMove(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const
{
  int presences,patient_1,patient_2,e_shift,count=0;
  do{
    mv.day = Random::Uniform<int>(0,in.Days()-1);
    e_shift=mv.day*3;
    presences=in.PatientsPres(e_shift);
    count++;
  }while (count<=100 && presences<2);
  if(count>=100)
        throw EmptyNeighborhood();
  else 
    count=0;
  patient_1=in.PatientInd(e_shift,Random::Uniform<int>(0,presences-1));
  do{
    patient_2=in.PatientInd(e_shift,Random::Uniform<int>(0,presences-1));
    count++;
  } while(count<100 && out.RoomAssignment(patient_1,mv.day)==out.RoomAssignment(patient_2,mv.day));
  if(count>=100)
    throw EmptyNeighborhood();
  if(patient_1<patient_2){
    mv.p1=patient_1;
    mv.p2=patient_2;
  }
  else{
    mv.p1=patient_2;
    mv.p2=patient_1;
  }
  mv.room_1=out.RoomAssignment(mv.p1,mv.day);
  mv.room_2=out.RoomAssignment(mv.p2,mv.day);
  mv.n11=out.NurseAssignment(mv.p1,e_shift);
  mv.n21=out.NurseAssignment(mv.p2,e_shift);
  mv.n12=out.NurseAssignment(mv.p1,e_shift+1);
  mv.n22=out.NurseAssignment(mv.p2,e_shift+1);
  mv.n13=out.NurseAssignment(mv.p1,e_shift+2);
  mv.n23=out.NurseAssignment(mv.p2,e_shift+2);
} 

bool IPRNPA_SwapRoomsNeighborhoodExplorer::FeasibleMove(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const
{
  return mv.room_1!=mv.room_2;
} 

void IPRNPA_SwapRoomsNeighborhoodExplorer::MakeMove(IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const
{ 
  out.SwapRooms(mv.day,mv.p1,mv.p2);
}  

void IPRNPA_SwapRoomsNeighborhoodExplorer::FirstMove(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const
{
  AnyFirstMove(out,mv);
  while (!FeasibleMove(out,mv))
  AnyNextMove(out,mv);	
}

void IPRNPA_SwapRoomsNeighborhoodExplorer::AnyFirstMove(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const
{
  mv.day=-1;
  int e_shift;
  do{
    mv.day++;
    e_shift=mv.day*3;
  }while(in.PatientsPres(e_shift)<2);
  mv.p1=in.PatientInd(e_shift,0);
  mv.p2=in.PatientInd(e_shift,1);
  mv.room_1=out.RoomAssignment(mv.p1,mv.day);
  mv.room_2=out.RoomAssignment(mv.p2,mv.day);
  mv.n11=out.NurseAssignment(mv.p1,e_shift);
  mv.n21=out.NurseAssignment(mv.p2,e_shift);
  mv.n12=out.NurseAssignment(mv.p1,e_shift+1);
  mv.n22=out.NurseAssignment(mv.p2,e_shift+1);
  mv.n13=out.NurseAssignment(mv.p1,e_shift+2);
  mv.n23=out.NurseAssignment(mv.p2,e_shift+2);
  mv.p1_idx=0;
  mv.p2_idx=1;
}

bool IPRNPA_SwapRoomsNeighborhoodExplorer::NextMove(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const
{
  do
    if (!AnyNextMove(out,mv))
      return false;
  while (!FeasibleMove(out,mv));
  return true;
}

bool IPRNPA_SwapRoomsNeighborhoodExplorer::AnyNextMove(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const
{
   if (NextPatientTwo(out,mv)) 
    {
      return true;
    }
  else if (NextPatientOne(out,mv)) 
    {
      FirstPatientTwo(out,mv); 
      return true;
    }
  else if (mv.day < in.Days() - 1) 
    {
      mv.day++;
      FirstPatientPair(out,mv);
      return true;
    }
  else return false;
}

bool IPRNPA_SwapRoomsNeighborhoodExplorer::NextPatientOne(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const{
  int e_shift=mv.day*3,presences=in.PatientsPres(e_shift);
  if(mv.p1_idx==presences-2)
    return false;
  mv.p1_idx++;
  mv.p1=in.PatientInd(e_shift,mv.p1_idx);
  mv.room_1=out.RoomAssignment(mv.p1,mv.day);
  mv.n11=out.NurseAssignment(mv.p1,e_shift);
  mv.n12=out.NurseAssignment(mv.p1,e_shift+1);
  mv.n13=out.NurseAssignment(mv.p1,e_shift+2);
  return true;
}

bool IPRNPA_SwapRoomsNeighborhoodExplorer::NextPatientTwo(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const{
  int e_shift=mv.day*3,presences=in.PatientsPres(e_shift);
  if(mv.p2_idx==presences-1)
    return false;
  mv.p2_idx++;
  mv.p2=in.PatientInd(e_shift,mv.p2_idx);
  mv.room_2=out.RoomAssignment(mv.p2,mv.day);
  mv.n21=out.NurseAssignment(mv.p2,e_shift);
  mv.n22=out.NurseAssignment(mv.p2,e_shift+1);
  mv.n23=out.NurseAssignment(mv.p2,e_shift+2);
  return true; //we assume there are at least two nurses working a shift s
}

void IPRNPA_SwapRoomsNeighborhoodExplorer::FirstPatientPair(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const{
  int e_shift=mv.day*3;
  mv.p1=in.PatientInd(e_shift,0);
  mv.p2=in.PatientInd(e_shift,1);
  mv.room_1=out.RoomAssignment(mv.p1,mv.day);
  mv.room_2=out.RoomAssignment(mv.p2,mv.day);
  mv.n11=out.NurseAssignment(mv.p1,e_shift);
  mv.n21=out.NurseAssignment(mv.p2,e_shift);
  mv.n12=out.NurseAssignment(mv.p1,e_shift+1);
  mv.n22=out.NurseAssignment(mv.p2,e_shift+1);
  mv.n13=out.NurseAssignment(mv.p1,e_shift+2);
  mv.n23=out.NurseAssignment(mv.p2,e_shift+2);
  mv.p1_idx=0;
  mv.p2_idx=1;
}

void IPRNPA_SwapRoomsNeighborhoodExplorer::FirstPatientTwo(const IPRNPA_Output& out, IPRNPA_SwapRooms& mv) const{
  int e_shift=mv.day*3;
  mv.p2_idx=mv.p1_idx+1;
  mv.p2=in.PatientInd(e_shift,mv.p2_idx);
  mv.room_2=out.RoomAssignment(mv.p2,mv.day);
  mv.n22=out.NurseAssignment(mv.p2,e_shift+1);
  mv.n13=out.NurseAssignment(mv.p1,e_shift+2);
  mv.n23=out.NurseAssignment(mv.p2,e_shift+2);
}

/*****************************************************************************
 * IPRNPA_ChangeNurse Deltas
 *****************************************************************************/

cost_type IPRNPA_ChangeNurseDeltaContinuity::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const{
  int cost=0;
  if(out.EverAssigned(mv.patient,mv.old_nurse)==1 && out.EverAssigned(mv.patient,mv.new_nurse)>0)
    cost--;
  else if(out.EverAssigned(mv.patient,mv.old_nurse)>1 && out.EverAssigned(mv.patient,mv.new_nurse)==0)
    cost++;
  return cost;
}

cost_type IPRNPA_ChangeNurseDeltaSkill::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const
{ 
  int cost = 0;
  if((mv.shift%3)!=2){
    unsigned admission=in.GetPatient(mv.patient).admission-1;
    unsigned old_skill_level=in.GetNurse(mv.old_nurse).skill_lev, new_skill_level=in.GetNurse(mv.new_nurse).skill_lev, skill_req=in.GetPatient(mv.patient).skill_req[mv.shift-admission];
    if(old_skill_level!=new_skill_level && skill_req>0){
      if(old_skill_level<skill_req && new_skill_level>=skill_req)
        cost--;
      else if(old_skill_level>=skill_req && new_skill_level<skill_req)
        cost++; 
    }
  }
  return cost;
}

cost_type IPRNPA_ChangeNurseDeltaExcess::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const //NB cost multiplied by 10
{
  float cost = 0;
  int is_old=in.SearchWs(mv.old_nurse,mv.shift);
  int is_new=in.SearchWs(mv.new_nurse,mv.shift);
  float temp_cost=0,proj_load=0,mload_old=in.GetNurse(mv.old_nurse).max_loads[is_old],mload_new=in.GetNurse(mv.new_nurse).max_loads[is_new];
  float wp=in.GetPatient(mv.patient).workload_req[mv.shift-(in.GetPatient(mv.patient).admission-1)];
  if(mload_old<out.Load(mv.old_nurse,is_old)){
    proj_load=out.Load(mv.old_nurse,is_old)-wp;
    if(proj_load>=mload_old)
      temp_cost-=wp;
    else
      temp_cost-=wp-(mload_old-proj_load);
  }
  if(mload_new<out.Load(mv.new_nurse,is_new)+wp){ //we have a new violation
    if(mload_new<=out.Load(mv.new_nurse,is_new))
      temp_cost+=wp;
    else
      temp_cost+=wp-(mload_new-out.Load(mv.new_nurse,is_new));
  }
  cost=temp_cost;
  return cost;
}

cost_type IPRNPA_ChangeNurseDeltaTotalBalance::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const
{ 
  cost_type cost=0;
  int old_ws=in.SearchWs(mv.old_nurse,mv.shift),new_ws=in.SearchWs(mv.new_nurse,mv.shift);
  double load_p=in.GetPatient(mv.patient).workload_req[mv.shift-(in.GetPatient(mv.patient).admission-1)],cost_removal=0,cost_insert=0,cost_pair=0;
  float mload_old=in.GetNurse(mv.old_nurse).max_loads[old_ws],mload_new=in.GetNurse(mv.new_nurse).max_loads[new_ws];
  for(unsigned i=0;i<in.Nurses();i++){
    if(i!= mv.old_nurse && i!=mv.new_nurse){
      cost_removal+=abs((out.TotalLoad(mv.old_nurse)-(load_p/mload_old))-out.TotalLoad(i))-abs(out.TotalLoad(mv.old_nurse)-out.TotalLoad(i));
      cost_insert+=abs((out.TotalLoad(mv.new_nurse)+(load_p/mload_new))-out.TotalLoad(i))-abs(out.TotalLoad(mv.new_nurse)-out.TotalLoad(i));
    }
  }
  cost_pair=abs((out.TotalLoad(mv.old_nurse)-(load_p/mload_old))-(out.TotalLoad(mv.new_nurse)+(load_p/mload_new)))-abs(out.TotalLoad(mv.old_nurse)-out.TotalLoad(mv.new_nurse));
  cost=(cost_removal+cost_insert+cost_pair);
  return cost;
}
//TODO possibily add a data structure that associates to each shift a vector of nurses to reduce complexity, however TotalBalance is 0(n) anyway
cost_type IPRNPA_ChangeNurseDeltaShiftBalance::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const
{ 
  cost_type cost=0;
  int old_ws=in.SearchWs(mv.old_nurse,mv.shift),new_ws=in.SearchWs(mv.new_nurse,mv.shift),cur_ws;
  double load_p=in.GetPatient(mv.patient).workload_req[mv.shift-(in.GetPatient(mv.patient).admission-1)],cost_removal=0,cost_insert=0,cost_pair=0;
  float mload_old=in.GetNurse(mv.old_nurse).max_loads[old_ws],mload_new=in.GetNurse(mv.new_nurse).max_loads[new_ws],mload_curr;
  double rel_load_old=out.Load(mv.old_nurse,old_ws)/mload_old,rel_load_new=out.Load(mv.new_nurse,new_ws)/mload_new,rel_load_cur;
  for(unsigned i=0;i<in.Nurses();i++){
    if(i!= mv.old_nurse && i!=mv.new_nurse && in.NurseWorking(i,mv.shift)){
      cur_ws=in.SearchWs(i,mv.shift);
      mload_curr=in.GetNurse(i).max_loads[cur_ws];
      rel_load_cur=out.Load(i,cur_ws)/mload_curr;
      cost_removal+=abs(((out.Load(mv.old_nurse,old_ws)-load_p)/mload_old)-rel_load_cur)-abs(rel_load_old-rel_load_cur);
      cost_insert+=abs(((out.Load(mv.new_nurse,new_ws)+load_p)/mload_new)-rel_load_cur)-abs(rel_load_new-rel_load_cur);
    }
  }
  cost_pair=abs(((out.Load(mv.old_nurse,old_ws)-load_p)/mload_old)-(out.Load(mv.new_nurse,new_ws)+load_p)/mload_new)-abs(rel_load_old-rel_load_new);
  cost=cost_removal+cost_insert+cost_pair;
  return cost;
}

cost_type IPRNPA_ChangeNurseDeltaNursesperRoom::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const //NB cost multiplied by 10
{
  int cost=0;
  int room=out.RoomAssignment(mv.patient,mv.shift/3),old_ws=in.SearchWs(mv.old_nurse,mv.shift),new_ws=in.SearchWs(mv.new_nurse,mv.shift);
  if(out.NurseinRoom(mv.old_nurse,old_ws,room)==1)
    cost--;
  if(out.NurseinRoom(mv.new_nurse,new_ws,room)==0)
    cost++;
  return cost;
}

cost_type IPRNPA_ChangeNurseDeltaWalkingCircle::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const{ 
  cost_type cost=0;
  int room=out.RoomAssignment(mv.patient,mv.shift/3),old_ws=in.SearchWs(mv.old_nurse,mv.shift),new_ws=in.SearchWs(mv.new_nurse,mv.shift);
  float c_weight=in.GetShift(mv.shift).c_weight;
  unsigned i;
  if(out.NurseinRoom(mv.old_nurse,old_ws,room)==1){
    for(i=0;i<in.Rooms();i++){
      if(room!=i && out.NurseinRoom(mv.old_nurse,old_ws,i)>0){
        cost-=((in.GetDistance(room+1,i+1)+in.GetDistance(i+1,room+1))*c_weight)/2;
      }
    }
  }
  if(out.NurseinRoom(mv.new_nurse,new_ws,room)==0){
    for(i=0;i<in.Rooms();i++){
      if(out.NurseinRoom(mv.new_nurse,new_ws,i)>0)
        cost+=((in.GetDistance(room+1,i+1)+in.GetDistance(i+1,room+1))*c_weight)/2;
    }
  }
  return cost;
}
//TODO discrepancy between cost component and delta due to rounding
cost_type IPRNPA_ChangeNurseDeltaWalkingStar::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeNurse& mv) const{ 
  cost_type cost=0;
  int room=out.RoomAssignment(mv.patient,mv.shift/3),old_ws=in.SearchWs(mv.old_nurse,mv.shift),new_ws=in.SearchWs(mv.new_nurse,mv.shift);
  float s_weight=in.GetShift(mv.shift).s_weight;
  if(out.NurseinRoom(mv.old_nurse,old_ws,room)==1){
    cost-=in.GetDistance(0,room+1)*s_weight;
  }
  if(out.NurseinRoom(mv.new_nurse,new_ws,room)==0){
    cost+=in.GetDistance(0,room+1)*s_weight;
  }
  return cost;
}

/*****************************************************************************
 * IPRNPA_ChangeRoom
 *****************************************************************************/

cost_type IPRNPA_ChangeRoomDeltaTransfer::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const{ 
  int cost=0;
  if(mv.day>0){
    if(out.RoomAssignment(mv.patient,mv.day-1)==mv.old_room) // a transfer will be added
      cost++;
    else if (out.RoomAssignment(mv.patient,mv.day-1)==mv.new_room)
      cost--;
  }
  if(mv.day<in.Days()-1){
    if(out.RoomAssignment(mv.patient,mv.day+1)==mv.old_room) // a transfer will be added
      cost++;
    else if(out.RoomAssignment(mv.patient,mv.day+1)==mv.new_room) // a transfer will be added
      cost--;
  } 
  return cost;
}

cost_type IPRNPA_ChangeRoomDeltaAgeGroup::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const{ 
  int cost=0;
  int temp=-1;
  unsigned i;
  /*removal: if patient is the only min or the only max than we need to recalculate agedifference*/
  if(out.Occupancy(mv.old_room,mv.day)>1){ //at least two patients (including mv.patient) in the old_room
    //cerr<<"Agegroup of patient "<<in.GetPatient(mv.patient).agegroup<<" agemin_room "<<out.AgeMin(mv.old_room,mv.day)<<" agemax_room "<<out.AgeMax(mv.old_room,mv.day)<<endl;
    if(in.GetPatient(mv.patient).agegroup==out.AgeMin(mv.old_room,mv.day) && out.NumMin(mv.old_room,mv.day)==1){
      for(i=0;i<out.Occupancy(mv.old_room,mv.day);i++){
        if(temp==-1 && out.GetOccupant(mv.old_room,mv.day,i)!=mv.patient)
          temp=in.GetPatient(out.GetOccupant(mv.old_room,mv.day,i)).agegroup;
        else if(out.GetOccupant(mv.old_room,mv.day,i)!=mv.patient && in.GetPatient(out.GetOccupant(mv.old_room,mv.day,i)).agegroup<temp)
          temp=in.GetPatient(out.GetOccupant(mv.old_room,mv.day,i)).agegroup;  
      }
      //cout<<"New Min "<<temp<<" old min "<<in.GetPatient(mv.patient).agegroup<<endl;
      cost-=temp-in.GetPatient(mv.patient).agegroup;
      ////cerr<<"Entering this first if"<<endl;
    }
    else if(in.GetPatient(mv.patient).agegroup==out.AgeMax(mv.old_room,mv.day) && out.NumMax(mv.old_room,mv.day)==1){
      for(i=0;i<out.Occupancy(mv.old_room,mv.day);i++){
        if(temp==-1 && out.GetOccupant(mv.old_room,mv.day,i)!=mv.patient)
          temp=in.GetPatient(out.GetOccupant(mv.old_room,mv.day,i)).agegroup;
        else if((out.GetOccupant(mv.old_room,mv.day,i)!=mv.patient  && in.GetPatient(out.GetOccupant(mv.old_room,mv.day,i)).agegroup>temp))
          temp=in.GetPatient(out.GetOccupant(mv.old_room,mv.day,i)).agegroup;  
      }
      //cerr<<"New Max "<<temp<<" old max "<<in.GetPatient(mv.patient).agegroup<<endl;
      cost-=in.GetPatient(mv.patient).agegroup-temp;
      ////cerr<<"Entering this second if"<<endl;
    }
    //cerr<<"Cost after removal "<<cost<<"-->";
  }
  if(out.Occupancy(mv.new_room,mv.day)>0){ 
    //cerr<<"New room has bounds "<<out.AgeMin(mv.new_room,mv.day)<<"--"<<out.AgeMax(mv.new_room,mv.day)<<endl;
    if(in.GetPatient(mv.patient).agegroup<out.AgeMin(mv.new_room,mv.day)){
      cost+=out.AgeMin(mv.new_room,mv.day)-in.GetPatient(mv.patient).agegroup;
      ////cerr<<"Modifying new_room bounds"<<endl;
    }
    else if(in.GetPatient(mv.patient).agegroup>out.AgeMax(mv.new_room,mv.day)){
      cost+=in.GetPatient(mv.patient).agegroup-out.AgeMax(mv.new_room,mv.day);
      ////cerr<<"Modifying new_room bounds max"<<endl;
    }
    //cerr<<"Cost after insert "<<cost<<endl;
  }
  return cost;
}

cost_type IPRNPA_ChangeRoomDeltaGender::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const{ 
  int cost=0;
  /*if 0 all men and removing does not improve situation, if num_female==num_occupants all women and removing does not improve situation*/
  if(out.Occupancy(mv.old_room,mv.day)>1){
    if(out.NumFemales(mv.old_room,mv.day)!=0 && out.NumFemales(mv.old_room,mv.day)!=out.Occupancy(mv.old_room,mv.day)){
      if(in.GetPatient(mv.patient).gender==Gender::female && out.NumFemales(mv.old_room,mv.day)==1)
        cost--;
      else if(in.GetPatient(mv.patient).gender==Gender::male && out.NumFemales(mv.old_room,mv.day)==out.Occupancy(mv.old_room,mv.day)-1)
        cost--;
    }
  }
  if(out.Occupancy(mv.new_room,mv.day)>0){
    if(out.NumFemales(mv.new_room,mv.day)==out.Occupancy(mv.new_room,mv.day) && in.GetPatient(mv.patient).gender==Gender::male)
      cost++;
    if(out.NumFemales(mv.new_room,mv.day)==0 && in.GetPatient(mv.patient).gender==Gender::female)
      cost++;
  }
  return cost;
}

cost_type IPRNPA_ChangeRoomDeltaEquipment::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const{ //possibly change the granularity after their answer
  int cost=0,mod=0;
  int sa=in.GetPatient(mv.patient).admission-1,cs,rs,ce;
  /*cost changes if for every shift of stay the patient requires an equipment and either a) old_room had the equipment and new_room doesn't OR b) old_room didn't have the equipment but new_room does */
  cs=mv.day*3;
  rs=cs-sa;             //relative shift w.r.t. vector of the patient
  if(in.GetPatient(mv.patient).equip_req[rs].size()!=0){ //don't know if it is needed
    for(unsigned e=0;e<in.GetPatient(mv.patient).equip_req[rs].size();e++){
      ce=in.GetPatient(mv.patient).equip_req[rs][e];
        //cerr<<"Current equipment required "<<ce<<" the old room is "<<mv.old_room<<" the new room is "<<mv.new_room<<endl;
        if(!in.RoomhasEquip(mv.old_room,ce) && !in.RoomhasEquip(mv.new_room,ce)) //at least one error persist, so even if another violation is corrected penalty stays
          return cost;
        else if(in.RoomhasEquip(mv.old_room,ce) && !in.RoomhasEquip(mv.new_room,ce))
          mod++;
        else if(!in.RoomhasEquip(mv.old_room,ce) && in.RoomhasEquip(mv.new_room,ce))
          mod--;
      }
  }
  if(mod>0)
    cost=1;
  else if (mod<0)
    cost=-1;
  return cost;
}

cost_type IPRNPA_ChangeRoomDeltaNursesperRoom::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const{
  int cost=0,nurse,ws;
  for(unsigned i=0;i<3;i++){
    nurse=out.NurseAssignment(mv.patient,(mv.day*3)+i);
    ws=in.SearchWs(nurse,(mv.day*3)+i);
    if(out.NurseinRoom(nurse,ws,mv.old_room)==1)
      cost--;
    if(out.NurseinRoom(nurse,ws,mv.new_room)==0)
      cost++;
  }
  return cost;
}

cost_type IPRNPA_ChangeRoomDeltaWalkingCircle::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const{
  double cost=0;
  int nurse,cs,ws,i,j;
  float c_weight;
  for(j=0;j<3;j++){
    cs=(mv.day*3)+j;
    nurse=out.NurseAssignment(mv.patient,cs);
    ws=in.SearchWs(nurse,cs);
    c_weight=in.GetShift(cs).c_weight;
    //TODO:reduce distances between rooms but eventually add the distance between the old_room and the new_room
    if(out.NurseinRoom(nurse,ws,mv.old_room)==1){
      for(i=0;i<in.Rooms();i++){
        if(out.NurseinRoom(nurse,ws,i)>0){
          if(mv.new_room!=i){
            cost-=((in.GetDistance(mv.old_room+1,i+1)+in.GetDistance(i+1,mv.old_room+1))*c_weight)/2;
          }
          else if(out.NurseinRoom(nurse,ws,mv.new_room)>0)
            cost-=((in.GetDistance(mv.old_room+1,mv.new_room+1)+in.GetDistance(mv.new_room+1,mv.old_room+1))*c_weight)/2;
        }
      }
    }
    if(out.NurseinRoom(nurse,ws,mv.new_room)==0){
      for(i=0;i<in.Rooms();i++){
        if(out.NurseinRoom(nurse,ws,i)>0){
          if(i!=mv.old_room){
            cost+=((in.GetDistance(mv.new_room+1,i+1)+in.GetDistance(i+1,mv.new_room+1))*c_weight)/2;
          }
          else if(out.NurseinRoom(nurse,ws,mv.old_room)>1)
            cost+=((in.GetDistance(mv.new_room+1,mv.old_room+1)+in.GetDistance(mv.old_room+1,mv.new_room+1))*c_weight)/2;
        }
      }
    }
  }
  return cost;
}

cost_type IPRNPA_ChangeRoomDeltaWalkingStar::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_ChangeRoom& mv) const{
  double cost=0;
  int nurse,cs,ws;
  float s_weight;
  for(unsigned i=0;i<3;i++){
    cs=(mv.day*3)+i;
    nurse=out.NurseAssignment(mv.patient,cs);
    ws=in.SearchWs(nurse,cs);
    s_weight=in.GetShift(cs).s_weight;
    //cerr<<" It's NurseInRoom "<<endl;
    if(out.NurseinRoom(nurse,ws,mv.old_room)==1){
      cost-=in.GetDistance(0,mv.old_room+1)*s_weight;
    }
    if(out.NurseinRoom(nurse,ws,mv.new_room)==0){
      cost+=in.GetDistance(0,mv.new_room+1)*s_weight;
    }
  }
  return cost;
}

/*****************************************************************************
 * IPRNPA_SwapNurses
 *****************************************************************************/
cost_type IPRNPA_SwapNursesDeltaContinuity::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const{
  int cost=0;
  //TODO write a function,code is symmetric and can be used also for change nurse
  if(out.EverAssigned(mv.p1,mv.n1)==1 && out.EverAssigned(mv.p1,mv.n2)>0)
    cost--;
  else if(out.EverAssigned(mv.p1,mv.n1)>1 && out.EverAssigned(mv.p1,mv.n2)==0)
    cost++;
  if(out.EverAssigned(mv.p2,mv.n2)==1 && out.EverAssigned(mv.p2,mv.n1)>0)
    cost--;
  else if(out.EverAssigned(mv.p2,mv.n2)>1 && out.EverAssigned(mv.p2,mv.n1)==0)
    cost++;
  return cost;
}

cost_type IPRNPA_SwapNursesDeltaSkill::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const
{ 
  int cost = 0;
  if((mv.shift%3)!=2){
    unsigned p1_admission=in.GetPatient(mv.p1).admission-1,p2_admission=in.GetPatient(mv.p2).admission-1;;
    unsigned n1_skill_level=in.GetNurse(mv.n1).skill_lev,n2_skill_level=in.GetNurse(mv.n2).skill_lev;
    unsigned p1_skill_req=in.GetPatient(mv.p1).skill_req[mv.shift-p1_admission],p2_skill_req=in.GetPatient(mv.p2).skill_req[mv.shift-p2_admission];
    //TODO write a function,code is symmetric and can be used also for change nurse
    if(n1_skill_level!=n2_skill_level){  //otherwise everything stays as it is
    //consider p1
    if(n1_skill_level<p1_skill_req && n2_skill_level>=p1_skill_req)
      cost--;
    else if (n1_skill_level>=p1_skill_req && n2_skill_level<p1_skill_req)
      cost++;
    //consider p2
    if(n2_skill_level<p2_skill_req && n1_skill_level>=p2_skill_req)
      cost--;
    else if (n2_skill_level>=p2_skill_req && n1_skill_level<p2_skill_req)
      cost++;
    }
  }
  return cost;
}

cost_type IPRNPA_SwapNursesDeltaExcess::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const{

  float cost = 0;
  int is_n1=in.SearchWs(mv.n1,mv.shift);
  int is_n2=in.SearchWs(mv.n2,mv.shift);
  float proj_load_n1,proj_load_n2,mload_n1=in.GetNurse(mv.n1).max_loads[is_n1],mload_n2=in.GetNurse(mv.n2).max_loads[is_n2];
  float wp1=(in.GetPatient(mv.p1).workload_req[mv.shift-(in.GetPatient(mv.p1).admission-1)]);
  float wp2=(in.GetPatient(mv.p2).workload_req[mv.shift-(in.GetPatient(mv.p2).admission-1)]);
  if(wp1!=wp2){ //otherwise it makes no difference
    float wd=wp1-wp2;
    //rethink this: I should cover every possibility
    proj_load_n1=out.Load(mv.n1,is_n1)-wd;
    proj_load_n2=out.Load(mv.n2,is_n2)+wd;
    //take care of n1
    if(mload_n1<out.Load(mv.n1,is_n1)){ //there was a previous violation
      if(proj_load_n1>=mload_n1) //violation remains
        cost-=wd;
      else //There is no more violation 
        cost-=wd-(mload_n1-proj_load_n1);
    }
    else if(proj_load_n1>mload_n1) //there was not a violation but there is now
      cost-=wd+(mload_n1-out.Load(mv.n1,is_n1));
    //take care of n2
    if(mload_n2<out.Load(mv.n2,is_n2)){ //there was a previous violation
      if(proj_load_n2>=mload_n2) //violation remains
        cost+=wd;
      else //There is no more violation 
        cost+=wd+(mload_n2-proj_load_n2);
    }
    else if(proj_load_n2>mload_n2) //there was not a violation but there is now
      cost+=wd-(mload_n2-out.Load(mv.n2,is_n2));
  }
  return cost;
}

cost_type IPRNPA_SwapNursesDeltaTotalBalance::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const
{ 
  cost_type cost=0;
  int n1_ws=in.SearchWs(mv.n1,mv.shift),n2_ws=in.SearchWs(mv.n2,mv.shift);
  float load_p1=in.GetPatient(mv.p1).workload_req[mv.shift-(in.GetPatient(mv.p1).admission-1)],load_p2=in.GetPatient(mv.p2).workload_req[mv.shift-(in.GetPatient(mv.p2).admission-1)];
  float load_d=load_p1-load_p2;
  double cost_removal=0,cost_insert=0,cost_pair=0;
  float mload_n1=in.GetNurse(mv.n1).max_loads[n1_ws],mload_n2=in.GetNurse(mv.n2).max_loads[n2_ws];
  for(unsigned i=0;i<in.Nurses();i++){
    if(i!= mv.n1 && i!=mv.n2){
      cost_removal+=abs((out.TotalLoad(mv.n1)-(load_d/mload_n1))-out.TotalLoad(i))-abs(out.TotalLoad(mv.n1)-out.TotalLoad(i));
      cost_insert+=abs((out.TotalLoad(mv.n2)+(load_d/mload_n2))-out.TotalLoad(i))-abs(out.TotalLoad(mv.n2)-out.TotalLoad(i));
    }
  }
  cost_pair=abs((out.TotalLoad(mv.n1)-(load_d/mload_n1))-(out.TotalLoad(mv.n2)+(load_d/mload_n2)))-abs(out.TotalLoad(mv.n1)-out.TotalLoad(mv.n2));
  cost=(cost_removal+cost_insert+cost_pair);
  return cost;
}

cost_type IPRNPA_SwapNursesDeltaShiftBalance::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const{
  cost_type cost=0;
  int n1_ws=in.SearchWs(mv.n1,mv.shift),n2_ws=in.SearchWs(mv.n2,mv.shift),cur_ws;
  float load_p1=in.GetPatient(mv.p1).workload_req[mv.shift-(in.GetPatient(mv.p1).admission-1)];
  float load_p2=in.GetPatient(mv.p2).workload_req[mv.shift-(in.GetPatient(mv.p2).admission-1)];
  float load_d=load_p1-load_p2;
  double cost_n1=0,cost_n2=0,cost_pair=0;
  float mload_n1=in.GetNurse(mv.n1).max_loads[n1_ws],mload_n2=in.GetNurse(mv.n2).max_loads[n2_ws],mload_curr;
  double rel_load_n1=out.Load(mv.n1,n1_ws)/mload_n1,rel_load_n2=out.Load(mv.n2,n2_ws)/mload_n2,rel_load_cur;
  for(unsigned i=0;i<in.Nurses();i++){
    if(i!= mv.n1 && i!=mv.n2 && in.NurseWorking(i,mv.shift)){
      cur_ws=in.SearchWs(i,mv.shift);
      mload_curr=in.GetNurse(i).max_loads[cur_ws];
      rel_load_cur=out.Load(i,cur_ws)/mload_curr;
      cost_n1+=abs(((out.Load(mv.n1,n1_ws)-load_d)/mload_n1)-rel_load_cur)-abs(rel_load_n1-rel_load_cur);
      cost_n2+=abs(((out.Load(mv.n2,n2_ws)+load_d)/mload_n2)-rel_load_cur)-abs(rel_load_n2-rel_load_cur);
    }
  }
  cost_pair=abs(((out.Load(mv.n1,n1_ws)-load_d)/mload_n1)-(out.Load(mv.n2,n2_ws)+load_d)/mload_n2)-abs(rel_load_n1-rel_load_n2);
  cost=cost_n1+cost_n2+cost_pair;
  return cost;
}

cost_type IPRNPA_SwapNursesDeltaNursesperRoom::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const //NB cost multiplied by 10
{
  int cost=0;
  int room_one=out.RoomAssignment(mv.p1,mv.shift/3),room_two=out.RoomAssignment(mv.p2,mv.shift/3),n1_ws=in.SearchWs(mv.n1,mv.shift),n2_ws=in.SearchWs(mv.n2,mv.shift);
  if(room_one!=room_two){
    if(out.NurseinRoom(mv.n1,n1_ws,room_one)==1)
      cost--;
    if(out.NurseinRoom(mv.n2,n2_ws,room_one)==0)
      cost++;
    if(out.NurseinRoom(mv.n2,n2_ws,room_two)==1)
      cost--;
    if(out.NurseinRoom(mv.n1,n1_ws,room_two)==0)
      cost++;
  }
  return cost;
}

cost_type IPRNPA_SwapNursesDeltaWalkingCircle::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const{ 
  cost_type cost=0;
  int room_1=out.RoomAssignment(mv.p1,mv.shift/3),room_2=out.RoomAssignment(mv.p2,mv.shift/3),n1_ws=in.SearchWs(mv.n1,mv.shift),n2_ws=in.SearchWs(mv.n2,mv.shift);
  float c_weight=in.GetShift(mv.shift).c_weight;
  unsigned i;
  bool n1_flag=false,n2_flag=false;
  if(room_1!=room_2){ //otherwise no difference
    if(out.NurseinRoom(mv.n1,n1_ws,room_1)==1){
      n1_flag=true;
      for(i=0;i<in.Rooms();i++){
        if(room_1!=i && out.NurseinRoom(mv.n1,n1_ws,i)>0){
            cost-=((in.GetDistance(room_1+1,i+1)+in.GetDistance(i+1,room_1+1))*c_weight)/2;
        }
      }
    }
    if(out.NurseinRoom(mv.n1,n1_ws,room_2)==0){
      for(i=0;i<in.Rooms();i++){
        if(out.NurseinRoom(mv.n1,n1_ws,i)>0 && (!n1_flag || room_1!=i)){
          cost+=((in.GetDistance(room_2+1,i+1)+in.GetDistance(i+1,room_2+1))*c_weight)/2;
        }
      }
    }
    if(out.NurseinRoom(mv.n2,n2_ws,room_2)==1){
      n2_flag=true;
      for(i=0;i<in.Rooms();i++){
        if(room_2!=i && out.NurseinRoom(mv.n2,n2_ws,i)>0)
          cost-=((in.GetDistance(room_2+1,i+1)+in.GetDistance(i+1,room_2+1))*c_weight)/2;
      }
    }
    if(out.NurseinRoom(mv.n2,n2_ws,room_1)==0){
      for(i=0;i<in.Rooms();i++){
        if(out.NurseinRoom(mv.n2,n2_ws,i)>0 && (!n2_flag || room_2!=i))
          cost+=((in.GetDistance(room_1+1,i+1)+in.GetDistance(i+1,room_1+1))*c_weight)/2;
      }
    }
  }
  return cost;
}

cost_type IPRNPA_SwapNursesDeltaWalkingStar::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapNurses& mv) const{ 
  cost_type cost=0;
  int room_1=out.RoomAssignment(mv.p1,mv.shift/3),room_2=out.RoomAssignment(mv.p2,mv.shift/3),n1_ws=in.SearchWs(mv.n1,mv.shift),n2_ws=in.SearchWs(mv.n2,mv.shift);
  float s_weight=in.GetShift(mv.shift).s_weight;
  if(room_1!=room_2){
    if(out.NurseinRoom(mv.n1,n1_ws,room_1)==1)
      cost-=in.GetDistance(0,room_1+1)*s_weight;
    if(out.NurseinRoom(mv.n1,n1_ws,room_2)==0)
      cost+=in.GetDistance(0,room_2+1)*s_weight;
    if(out.NurseinRoom(mv.n2,n2_ws,room_2)==1)
      cost-=in.GetDistance(0,room_2+1)*s_weight;
    if(out.NurseinRoom(mv.n2,n2_ws,room_1)==0)
      cost+=in.GetDistance(0,room_1+1)*s_weight;
  }
  return cost;
}

/*****************************************************************************
 * IPRNPA_SwapRooms
 *****************************************************************************/
//An external function can be written,essentially is a changeroom delta applied to both patients
cost_type IPRNPA_SwapRoomsDeltaTransfer::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const{ 
  int cost=0;
  if(mv.day>0){
    if(out.RoomAssignment(mv.p1,mv.day-1)==mv.room_1) // a transfer will be added
      cost++;
    else if (out.RoomAssignment(mv.p1,mv.day-1)==mv.room_2)
      cost--;
    if(out.RoomAssignment(mv.p2,mv.day-1)==mv.room_2)
      cost++;
    else if(out.RoomAssignment(mv.p2,mv.day-1)==mv.room_1)
      cost--;
  }
  if(mv.day<in.Days()-1){
    if(out.RoomAssignment(mv.p1,mv.day+1)==mv.room_1) // a transfer will be added
      cost++;
    else if(out.RoomAssignment(mv.p1,mv.day+1)==mv.room_2) // a transfer will be added
      cost--;
    if(out.RoomAssignment(mv.p2,mv.day+1)==mv.room_2) // a transfer will be added
      cost++;
    else if(out.RoomAssignment(mv.p2,mv.day+1)==mv.room_1) // a transfer will be added
      cost--;
  } 
  return cost;
}

cost_type IPRNPA_SwapRoomsDeltaAgeGroup::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const{ 
  int cost=0;
  if(in.GetPatient(mv.p1).agegroup!=in.GetPatient(mv.p2).agegroup){
    unsigned age_n2=in.GetPatient(mv.p2).agegroup;
    unsigned age_n1=in.GetPatient(mv.p1).agegroup;
    int i,candidate=-1;
    if(out.Occupancy(mv.room_1,mv.day)>1){
      if(in.GetPatient(mv.p1).agegroup==out.AgeMin(mv.room_1,mv.day) && out.NumMin(mv.room_1,mv.day)==1){ //p1 was LB  
          for(i=0;i<out.Occupancy(mv.room_1,mv.day);i++){
            if(out.GetOccupant(mv.room_1,mv.day,i)!=mv.p1){
              if(candidate==-1 || in.GetPatient(out.GetOccupant(mv.room_1,mv.day,i)).agegroup<candidate)
                candidate=in.GetPatient(out.GetOccupant(mv.room_1,mv.day,i)).agegroup;  
            } 
          }
        if(age_n2<candidate)
          cost-=age_n2-in.GetPatient(mv.p1).agegroup;
        else if(age_n2>out.AgeMax(mv.room_1,mv.day))
          cost+=(in.GetPatient(mv.p1).agegroup-candidate)+(age_n2-out.AgeMax(mv.room_1,mv.day));
        else
          cost-=candidate-in.GetPatient(mv.p1).agegroup;
      }
      else if(in.GetPatient(mv.p1).agegroup==out.AgeMax(mv.room_1,mv.day) && out.NumMax(mv.room_1,mv.day)==1){
        for(i=0;i<out.Occupancy(mv.room_1,mv.day);i++){
          if(out.GetOccupant(mv.room_1,mv.day,i)!=mv.p1){
            if(candidate==-1 || in.GetPatient(out.GetOccupant(mv.room_1,mv.day,i)).agegroup>candidate)
              candidate=in.GetPatient(out.GetOccupant(mv.room_1,mv.day,i)).agegroup;  
          }
        }
        if(age_n2>candidate)
          cost-=in.GetPatient(mv.p1).agegroup-age_n2;
        else if(age_n2<out.AgeMin(mv.room_1,mv.day))
          cost+=(candidate-in.GetPatient(mv.p1).agegroup)+(out.AgeMin(mv.room_1,mv.day)-age_n2);
        else
          cost-=in.GetPatient(mv.p1).agegroup-candidate;
      }
      else{
        if(age_n2<out.AgeMin(mv.room_1,mv.day))
          cost+=out.AgeMin(mv.room_1,mv.day)-age_n2;
        if(age_n2>out.AgeMax(mv.room_1,mv.day))
          cost+=age_n2-out.AgeMax(mv.room_1,mv.day);
      }
    }
    candidate=-1;
    if(out.Occupancy(mv.room_2,mv.day)>1){
      if(in.GetPatient(mv.p2).agegroup==out.AgeMin(mv.room_2,mv.day) && out.NumMin(mv.room_2,mv.day)==1){ //p2 was LB  
          for(i=0;i<out.Occupancy(mv.room_2,mv.day);i++){
            if(out.GetOccupant(mv.room_2,mv.day,i)!=mv.p2){
              if(candidate==-1 || in.GetPatient(out.GetOccupant(mv.room_2,mv.day,i)).agegroup<candidate)
                candidate=in.GetPatient(out.GetOccupant(mv.room_2,mv.day,i)).agegroup;  
            } 
          }
          if(age_n1<candidate)
            cost-=age_n1-in.GetPatient(mv.p2).agegroup;
          else if(age_n1>out.AgeMax(mv.room_2,mv.day))
            cost+=(in.GetPatient(mv.p2).agegroup-candidate)+(age_n1-out.AgeMax(mv.room_2,mv.day));
          else
            cost-=candidate-in.GetPatient(mv.p2).agegroup;
      }
      else if(in.GetPatient(mv.p2).agegroup==out.AgeMax(mv.room_2,mv.day) && out.NumMax(mv.room_2,mv.day)==1){
        for(i=0;i<out.Occupancy(mv.room_2,mv.day);i++){
          if(out.GetOccupant(mv.room_2,mv.day,i)!=mv.p2){
            if(candidate==-1 || in.GetPatient(out.GetOccupant(mv.room_2,mv.day,i)).agegroup>candidate)
              candidate=in.GetPatient(out.GetOccupant(mv.room_2,mv.day,i)).agegroup;  
          }
        }
        if(age_n1>candidate)
          cost-=in.GetPatient(mv.p2).agegroup-age_n1;
        else if(age_n1<out.AgeMin(mv.room_2,mv.day))
          cost+=(candidate-in.GetPatient(mv.p2).agegroup)+(out.AgeMin(mv.room_2,mv.day)-age_n1);
        else
          cost-=in.GetPatient(mv.p2).agegroup-candidate;
      }
      else{
        if(age_n1<out.AgeMin(mv.room_2,mv.day))
          cost+=out.AgeMin(mv.room_2,mv.day)-age_n1;
        if(age_n1>out.AgeMax(mv.room_2,mv.day))
          cost+=age_n1-out.AgeMax(mv.room_2,mv.day);
      }
    }
  }
  return cost;
}

cost_type IPRNPA_SwapRoomsDeltaGender::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const{ 
  int cost=0;
  /*if 0 all men and removing does not improve situation, if num_female==num_occupants all women and removing does not improve situation*/
  if(in.GetPatient(mv.p1).gender!=in.GetPatient(mv.p2).gender){ 
    //something changes only if pair is made up of two different gender==>otherwise everything stays as it is
    /*if 0 all men and removing does not improve situation, if num_female==num_occupants all women and removing does not improve situation*/
    if(out.Occupancy(mv.room_1,mv.day)>1){
      if(out.NumFemales(mv.room_1,mv.day)!=0 && out.NumFemales(mv.room_1,mv.day)!=out.Occupancy(mv.room_1,mv.day)){ // i might fix previous violation
        if(in.GetPatient(mv.p1).gender==Gender::female && out.NumFemales(mv.room_1,mv.day)==1)
          cost--;
        else if(in.GetPatient(mv.p1).gender==Gender::male && out.NumFemales(mv.room_1,mv.day)==out.Occupancy(mv.room_1,mv.day)-1)
          cost--;
      }
      //new violations might arise
      if(in.GetPatient(mv.p2).gender==Gender::female && out.NumFemales(mv.room_1,mv.day)==0)
        cost++;
      else if (in.GetPatient(mv.p2).gender==Gender::male && out.NumFemales(mv.room_1,mv.day)==out.Occupancy(mv.room_1,mv.day))
        cost++;
     }
    //room_2 violations
    if(out.Occupancy(mv.room_2,mv.day)>1){
      if(out.NumFemales(mv.room_2,mv.day)!=0 && out.NumFemales(mv.room_2,mv.day)!=out.Occupancy(mv.room_2,mv.day)){ // i might fix previous violation
        if(in.GetPatient(mv.p2).gender==Gender::female && out.NumFemales(mv.room_2,mv.day)==1)
            cost--;
        else if(in.GetPatient(mv.p2).gender==Gender::male && out.NumFemales(mv.room_2,mv.day)==out.Occupancy(mv.room_2,mv.day)-1)
            cost--;
      }
      //new violations might arise
      if(out.NumFemales(mv.room_2,mv.day)==out.Occupancy(mv.room_2,mv.day) && in.GetPatient(mv.p1).gender==Gender::male)
        cost++;
      if(out.NumFemales(mv.room_2,mv.day)==0 && in.GetPatient(mv.p1).gender==Gender::female)
        cost++;
    }
  }
  return cost;
}

cost_type IPRNPA_SwapRoomsDeltaEquipment::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const{ //possibly change the granularity after their answer
  int cost=0,mod=0;
  int sa_p1=in.GetPatient(mv.p1).admission-1,sa_p2=in.GetPatient(mv.p2).admission-1,cs,rs_p1,rs_p2,ce_p1,ce_p2;
  /*cost changes if for every shift of stay the patient requires an equipment and either a) old-room had the equipment and new-room doesn't OR b) old_room didn't have the equipment but new_room does */
  cs=mv.day*3;
  rs_p1=cs-sa_p1;             //relative shift w.r.t. vector of the p1
  if(in.GetPatient(mv.p1).equip_req[rs_p1].size()!=0){ //don't know if it is needed
    for(unsigned e=0;e<in.GetPatient(mv.p1).equip_req[rs_p1].size();e++){
      ce_p1=in.GetPatient(mv.p1).equip_req[rs_p1][e];
        if(!in.RoomhasEquip(mv.room_1,ce_p1) && !in.RoomhasEquip(mv.room_2,ce_p1)){ //at least one error persist, so even if another violation is corrected penalty stays
          mod=0;
          break;
        }
        else if(in.RoomhasEquip(mv.room_1,ce_p1) && !in.RoomhasEquip(mv.room_2,ce_p1))
          mod++;
        else if(!in.RoomhasEquip(mv.room_1,ce_p1) && in.RoomhasEquip(mv.room_2,ce_p1))
          mod--;
      }
  }
  if(mod>0)
    cost=1;
  else if (mod<0)
    cost=-1;
  //symmetric with p2
  mod=0;
  rs_p2=cs-sa_p2;
  if(in.GetPatient(mv.p2).equip_req[rs_p2].size()!=0){ //don't know if it is needed
    for(unsigned e=0;e<in.GetPatient(mv.p2).equip_req[rs_p2].size();e++){
      ce_p2=in.GetPatient(mv.p2).equip_req[rs_p2][e];
        if(!in.RoomhasEquip(mv.room_2,ce_p2) && !in.RoomhasEquip(mv.room_1,ce_p2)) //at least one error persist, so even if another violation is corrected penalty stays
          return cost;
        else if(in.RoomhasEquip(mv.room_2,ce_p2) && !in.RoomhasEquip(mv.room_1,ce_p2))
          mod++;
        else if(!in.RoomhasEquip(mv.room_2,ce_p2) && in.RoomhasEquip(mv.room_1,ce_p2))
          mod--;
      }
  }
  if(mod>0)
    cost+=1;
  else if (mod<0)
    cost+=-1;
  return cost;
}

//interaction between two swapped nurses?
cost_type IPRNPA_SwapRoomsDeltaNursesperRoom::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const{
  int cost=0; 
  //something changes only if pair is made up of two different nurses==>otherwise everything stays as it is
  int n1,n2,n1_ws,n2_ws,e_shift=mv.day*3;
  //something changes only if pair is made up of two different nurses==>otherwise everything stays as it is
  for(unsigned i=0;i<3;i++){
    n1=out.NurseAssignment(mv.p1,e_shift+i);
    n2=out.NurseAssignment(mv.p2,e_shift+i);
    if(n1!=n2){
      n1_ws=in.SearchWs(n1,e_shift+i);
      n2_ws=in.SearchWs(n2,e_shift+i);
      if(out.NurseinRoom(n1,n1_ws,mv.room_1)==1)
        cost--;
      if(out.NurseinRoom(n1,n1_ws,mv.room_2)==0)
        cost++;
      if(out.NurseinRoom(n2,n2_ws,mv.room_2)==1)
        cost--;
      if(out.NurseinRoom(n2,n2_ws,mv.room_1)==0)
        cost++;
    }
  }
  return cost;
}

//can be computed separately for each nurse (i.e. write an external function)
cost_type IPRNPA_SwapRoomsDeltaWalkingCircle::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const{
  double cost=0;
  int n1,n2,n1_ws,n2_ws,i,j,cs;
  float c_weight;
  for(j=0;j<3;j++){
    cs=(mv.day*3)+j;
    n1=out.NurseAssignment(mv.p1,cs);
    n2=out.NurseAssignment(mv.p2,cs);
    if(n1!=n2){
      n1_ws=in.SearchWs(n1,cs);
      n2_ws=in.SearchWs(n2,cs);
      c_weight=in.GetShift(cs).c_weight;
      //TODO:write a function because code is symmetric
      if(out.NurseinRoom(n1,n1_ws,mv.room_1)==1){
        for(i=0;i<in.Rooms();i++){
          if(out.NurseinRoom(n1,n1_ws,i)>0){
            if(mv.room_2!=i){
              cost-=((in.GetDistance(mv.room_1+1,i+1)+in.GetDistance(i+1,mv.room_1+1))*c_weight)/2;
            }
            else if(out.NurseinRoom(n1,n1_ws,mv.room_2)>0)
              cost-=((in.GetDistance(mv.room_1+1,mv.room_2+1)+in.GetDistance(mv.room_2+1,mv.room_1+1))*c_weight)/2;
          }
        }
      }
      if(out.NurseinRoom(n1,n1_ws,mv.room_2)==0){
        for(i=0;i<in.Rooms();i++){
          if(out.NurseinRoom(n1,n1_ws,i)>0){
            if(i!=mv.room_1){
              cost+=((in.GetDistance(mv.room_2+1,i+1)+in.GetDistance(i+1,mv.room_2+1))*c_weight)/2;
            }
            else if(out.NurseinRoom(n1,n1_ws,mv.room_1)>1)
              cost+=((in.GetDistance(mv.room_2+1,mv.room_1+1)+in.GetDistance(mv.room_1+1,mv.room_2+1))*c_weight)/2;
          }
        }
      }
      if(out.NurseinRoom(n2,n2_ws,mv.room_2)==1){
        for(i=0;i<in.Rooms();i++){
          if(out.NurseinRoom(n2,n2_ws,i)>0){
            if(mv.room_1!=i){
              cost-=((in.GetDistance(mv.room_2+1,i+1)+in.GetDistance(i+1,mv.room_2+1))*c_weight)/2;
            }
            else if(out.NurseinRoom(n2,n2_ws,mv.room_1)>0)
              cost-=((in.GetDistance(mv.room_1+1,mv.room_2+1)+in.GetDistance(mv.room_2+1,mv.room_1+1))*c_weight)/2;
          }
        }
      }
      if(out.NurseinRoom(n2,n2_ws,mv.room_1)==0){
        for(i=0;i<in.Rooms();i++){
          if(out.NurseinRoom(n2,n2_ws,i)>0){
            if(i!=mv.room_2){
              cost+=((in.GetDistance(mv.room_1+1,i+1)+in.GetDistance(i+1,mv.room_1+1))*c_weight)/2;
            }
            else if(out.NurseinRoom(n2,n2_ws,mv.room_2)>1)
              cost+=((in.GetDistance(mv.room_2+1,mv.room_1+1)+in.GetDistance(mv.room_1+1,mv.room_2+1))*c_weight)/2;
          }
        }
      }
    }
  }
  return cost;
}
cost_type IPRNPA_SwapRoomsDeltaWalkingStar::ComputeDeltaCost(const IPRNPA_Output& out, const IPRNPA_SwapRooms& mv) const{
  double cost=0;
  //something changes only if pair is made up of two different nurses==>otherwise everything stays as it is
  int n1,n2,n1_ws,n2_ws,cs;
  float s_weight;
  for(unsigned i=0;i<3;i++){
    cs=(mv.day*3)+i;
    n1=out.NurseAssignment(mv.p1,cs);
    n2=out.NurseAssignment(mv.p2,cs);
    if(n1!=n2){
      n1_ws=in.SearchWs(n1,cs);
      n2_ws=in.SearchWs(n2,cs);
      s_weight=in.GetShift(cs).s_weight;
      if(out.NurseinRoom(n1,n1_ws,mv.room_1)==1)
        cost-=in.GetDistance(0,mv.room_1+1)*s_weight;
      if(out.NurseinRoom(n1,n1_ws,mv.room_2)==0)
        cost+=in.GetDistance(0,mv.room_2+1)*s_weight;
      if(out.NurseinRoom(n2,n2_ws,mv.room_2)==1)
        cost-=in.GetDistance(0,mv.room_2+1)*s_weight;
      if(out.NurseinRoom(n2,n2_ws,mv.room_1)==0)
        cost+=in.GetDistance(0,mv.room_1+1)*s_weight;
    }
  }
  return cost;
}
