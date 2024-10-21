// File IPRNPA_Input.cc
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "json.hpp"
#include "IPRNPA_Input.hh"

ostream& operator<<(ostream& os, const Patient& p)
{
  os <<" gen:"<< (p.gender == Gender::female ? "F" : "M") << " agegr:" 
     <<  p.agegroup << " " 
     << p.admission << "-" 
     << p.discharge << " " 
     << p.duration << " " << endl;
  for(unsigned i=0;i<p.duration;i++){
    os<<" Shift "<< p.admission+i<< " s="<<p.skill_req[i]<<", w="<<p.workload_req[i]<<" e=[";
    for(unsigned j=0;j<p.equip_req[i].size();j++){
      os<<p.equip_req[i][j]<<" ";
    }
    os<<"];"<<endl;
    if(p.border){
      os<<"Border data present r:" <<p.current_room<<endl;
    }
  }
  return os;
}

ostream& operator<<(ostream& os, const Room& r)
{
  os<<"cap: "<<r.capacity<<" "<<"equip: ";
  for(unsigned i=0;i<r.equipments.size();i++)
    os<<r.equipments[i]<<" ";
  return os;
}

ostream& operator<<(ostream& os, const Nurse& n)
{
  os<<"sk: "<<n.skill_lev<<" "<<"working shifts: ";
  for(unsigned i=0;i<n.working_shifts.size();i++)
    os<<"("<<n.working_shifts[i]<<"-"<<n.max_loads[i]<<") ";
  return os;
}

IPRNPA_Input::IPRNPA_Input(string file_name)
{  
  unsigned p, r, e, n, s, ns, cs;
  string id, gender;
  ifstream is(file_name);
  if(!is)
  {
    cerr << "Cannot open input file " <<  file_name << endl;
    exit(1);
  }
  add_rooms=1;
  index_one=false;
  nlohmann::json j_in, j_p, j_r, j_s, j_n;
  is >> j_in;

  // read scalars
  patients = j_in["patients"].size();
  nurses = j_in["nurses"].size(); 
  rooms = j_in["rooms"].size(); 
  shifts = j_in["shifts"].size();
  if (j_in["lastDay"].is_null())
    days = shifts/3;
  else
     days = j_in["lastDay"];
  equipments = j_in["equipment"].size();
  skills = j_in["skillLevels"].size();

  // resize vectors
  distance.resize(rooms+1,vector<float>(rooms+1,0));
  nurses_vect.resize(nurses);
  patients_vect.resize(patients);
  shifts_vect.resize(shifts);
  rooms_vect.resize(rooms);
  equipments_vect.resize(equipments);
  nurse_works.resize(nurses,vector<bool>(shifts,false));
  patients_present.resize(shifts,vector<int>(0));
  
  for (e = 0; e < equipments; e++)
  {
    equipments_vect[e] = j_in["equipment"][e];
  }

  for (p = 0; p < patients; p++)
  {
    j_p = j_in["patients"][p];
    id = j_p["id"];
    if (stoi(id) != p+1)
      throw invalid_argument("patient id not sequential");
    patients_vect[p].agegroup = j_p["ageGroup"];
    patients_vect[p].admission = j_p["admission"];
    patients_vect[p].discharge = j_p["discharge"];
    for(s=patients_vect[p].admission-1;s<patients_vect[p].discharge;s++)
      patients_present[s].push_back(p);
    patients_vect[p].duration = patients_vect[p].discharge - patients_vect[p].admission+1;
    gender = j_p["gender"];
    patients_vect[p].gender = (gender == "F" ? Gender::female : Gender::male);
    patients_vect[p].skill_req.resize(patients_vect[p].duration);
    patients_vect[p].workload_req.resize(patients_vect[p].duration);
    patients_vect[p].equip_req.resize(patients_vect[p].duration);
    for (s = 0; s < patients_vect[p].duration; s++)
    {
      id = to_string(patients_vect[p].admission + s );
      patients_vect[p].skill_req[s] = j_p["skillReq"][id];
      patients_vect[p].workload_req[s] = j_p["workLoad"][id]; 
      unsigned requested_equipments = j_p["equipmentReq"][id].size();
      patients_vect[p].equip_req[s].resize(requested_equipments);
      for (e = 0; e < requested_equipments; e++)
      {
        patients_vect[p].equip_req[s][e] = FindEquipmentIndex(j_p["equipmentReq"][id][e]);
      }
    }
    if (!j_p["currentRoom"].is_null()){
      patients_vect[p].border=true;
      patients_vect[p].transfers=j_p["prevTransfers"];
      patients_vect[p].current_room=j_p["currentRoom"];
      patients_vect[p].prev_nurses.resize(j_p["prevAssignedNurses"].size());
      for(n=0;n<j_p["prevAssignedNurses"].size();n++)
        patients_vect[p].prev_nurses[n]=j_p["prevAssignedNurses"][n];
    }
    else
      patients_vect[p].border=false;
  }
  for(r=0;r<rooms;r++){
    j_r = j_in["rooms"][r];
    id = j_r["id"];
    if (stoi(id) != r+1)
      throw invalid_argument("room id not sequential");
    rooms_vect[r].capacity=j_r["capacity"];
    unsigned room_equipments=j_r["equipment"].size();
    rooms_vect[r].equipments.resize(room_equipments);
    for (e = 0; e < room_equipments; e++)
    {
      rooms_vect[r].equipments[e] = FindEquipmentIndex(j_r["equipment"][e]);
    }
  }
  for(s=0;s<shifts;s++){
    id=to_string(s+1);
    j_s=j_in["shifts"][id];
    shifts_vect[s].c_weight=j_s["circleWeight"];
    shifts_vect[s].s_weight=j_s["starWeight"];
  }
  for(n=0;n<nurses;n++){
    j_n = j_in["nurses"][n];
    id = j_n["id"];
    if (stoi(id) != n) 
      index_one=true;
    nurses_vect[n].skill_lev=j_n["skillLevel"];
    ns=j_n["workingShifts"].size();
    nurses_vect[n].working_shifts.resize(ns);
    nurses_vect[n].max_loads.resize(ns);
    for(s=0;s<ns;s++){
      cs=j_n["workingShifts"][s];
      nurses_vect[n].working_shifts[s]=cs-1;
      nurse_works[n][cs-1]=true;
      nurses_vect[n].max_loads[s]=j_n["maxLoad"][to_string(cs)];
    }
  }
  for(r=0;r<rooms+1;r++){
    for(p=0;p<rooms+1;p++){
      distance[r][p]=j_in["distances"][to_string(r)][to_string(p)];
    }
  }
  if (j_in["additionalRooms"].size()!=1)
    cerr<<"More than one additional room "<<endl;
}
ostream& operator<<(ostream& os, const IPRNPA_Input& in)
{
  unsigned i,j;
  os<<"Patients "<<in.patients<<", nurses "<<in.nurses<<", rooms "<<in.rooms<<", days "<<in.days<<", equipments "<<in.equipments<<", shifts "<<in.shifts<<",skills "<<in.skills<<endl;
  os<<"List of equipments: ";
  for(i=0;i<in.equipments;i++)
    os<<in.equipments_vect[i]<<" ";
  os<<endl;
  os<<"List of rooms:"<<endl;
  for(i=0;i<in.rooms;i++)
    os<<"Rm "<<i+1<<" "<<in.rooms_vect[i] << endl;
  os<<"List of shifts:"<<endl;
  for(i=0;i<in.shifts;i++)
    os<<"Sh "<<i+1<<" "<<in.shifts_vect[i].c_weight<<" - "<<in.shifts_vect[i].s_weight<<endl;
  os<<"List of nurses: "<<endl;
  for(i=0;i<in.nurses;i++)
    os<<"Nu "<<i+in.index_one<<" "<<in.nurses_vect[i] << endl;
  for(i=0;i<in.patients;i++){
    os<<"Pt "<<i+1<<" "<<in.patients_vect[i]<<endl;
  }
  for(i=0;i<in.rooms+1;i++){
    for(j=0;j<in.rooms+1;j++)
      os<<in.distance[i][j]<<" ";
    os<<endl;
  }
  return os;
}

bool IPRNPA_Input::RoomhasEquip(unsigned r,unsigned e) const
{
  for (unsigned i=0;i<rooms_vect[r].equipments.size(); i++){
    if(rooms_vect[r].equipments[i]==e)
      return true;
  }
   return false;
}

int IPRNPA_Input::FindEquipmentIndex(string s) const
{
  for (unsigned e = 0; e < equipments; e++)
    if (s == equipments_vect[e])
      return e;
  return -1;
}

int IPRNPA_Input::SearchWs(unsigned n,unsigned s) const
{
  for(unsigned i=0; i<nurses_vect[n].working_shifts.size();i++){
    if(nurses_vect[n].working_shifts[i]==s)
      return i;
  }
  return -1;
}

