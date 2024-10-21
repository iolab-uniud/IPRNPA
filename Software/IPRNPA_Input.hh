// File IPRNPA_Input.hh
#ifndef IPRNPA_INPUT_HH
#define IPRNPA_INPUT_HH
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

using namespace std;

inline void RemoveElement(vector<int>& v, int e)
{ v.erase(remove(v.begin(), v.end(), e), v.end()); }

inline bool Member(const vector<int>& v, int e)
{ return find(v.begin(), v.end(), e) != v.end(); }

class Room{
  friend ostream& operator<<(ostream& os, const Room& p);
  public:
  unsigned Capacity() const { return capacity; }
  unsigned Id() const { return id; }
  unsigned Equipments() const { return equipments.size(); }
  unsigned Equipment(unsigned i) const { return equipments[i]; }
  unsigned id;
  unsigned capacity;
  vector<unsigned> equipments;
};

class Shift{
  public:
  float Cweight() const {return c_weight;}
  float Sweight() const {return s_weight;}
  float c_weight,s_weight;
};

class Nurse{
  friend ostream& operator<<(ostream& os, const Nurse& p);
  public:
  unsigned SkillLev() const { return skill_lev; }
  unsigned WorkShifts() const {return working_shifts.size();}
  unsigned WorkingShift(int i) const {return working_shifts[i];}
  unsigned MaxLoad(int s) const {return max_loads[s];}
  unsigned id;
  unsigned skill_lev;
  vector<unsigned> working_shifts;
  vector<float> max_loads;
};

enum class Gender { female, male };

class Patient{
  friend ostream& operator<<(ostream& os, const Patient& p);
  public:
  unsigned AdmissionDay() const {return admission;}
  unsigned DischargeDay() const {return discharge;}
  unsigned Duration() const {return duration;}
  unsigned SkillReq(int d) const {return skill_req[d];}
  unsigned WorklReq(int d) const {return workload_req[d];}
  unsigned id; 
  Gender gender; 
  unsigned agegroup,admission,discharge,duration;
  vector<unsigned> skill_req, workload_req;
  vector<vector<unsigned> >  equip_req; 
  // border data: not null in case stay has started before current horizon
  bool border;
  unsigned current_room,transfers;
  vector<unsigned> prev_nurses; 
};

class IPRNPA_Input 
{
  friend ostream& operator<<(ostream& os, const IPRNPA_Input& in);
public:
  IPRNPA_Input(string file_name);
  unsigned Patients() const { return patients; }
  unsigned Nurses() const { return nurses; }
  unsigned Rooms() const { return rooms; }
  unsigned Days() const { return days; }
  unsigned Equipments() const { return equipments; }
  unsigned Shifts() const { return shifts; }
  unsigned Skills() const {return skills;}
  unsigned AddRooms() const {return add_rooms;}
  float GetDistance(int i,int j) const {return distance[i][j];} 
  bool NurseWorking(int n,int s) const {return nurse_works[n][s];}
  int PatientInd(int s,int i) const {return patients_present[s][i];}
  int PatientsPres(int s) const {return patients_present[s].size();}
  bool IndexOffset() const {return index_one;}
  string FindEquipmentString(int i) const {return equipments_vect[i];}
  bool RoomhasEquip(unsigned r,unsigned e) const;
  int SearchWs(unsigned n,unsigned s) const;
  Patient GetPatient(int i) const {return patients_vect[i];}
  Nurse GetNurse(int i) const {return nurses_vect[i];}
  Room GetRoom(int i) const {return rooms_vect[i];}
  Shift GetShift(int i) const {return shifts_vect[i];}
 private:
  unsigned patients, nurses, rooms, days, equipments, shifts, skills, add_rooms;
  bool index_one;
  vector<vector<float> > distance; 
  vector<vector<bool> > nurse_works;
  vector<vector<int> > patients_present;
  vector<Nurse> nurses_vect;
  vector<Patient> patients_vect;
  vector<Shift> shifts_vect;
  vector<Room> rooms_vect;
  vector<string> equipments_vect;
  // auxiliary methods
  int FindEquipmentIndex(string s) const;
};
#endif
