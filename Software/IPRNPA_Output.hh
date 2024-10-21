// File IPRNPA_Output.hh
#ifndef IPRNPA_OUTPUT_HH
#define IPRNPA_OUTPUT_HH
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "IPRNPA_Input.hh"

using namespace std;

class IPRNPA_Output 
{
  friend ostream& operator<<(ostream& os, const IPRNPA_Output& out);
  friend istream& operator>>(istream& is, IPRNPA_Output& out);
  friend bool operator==(const IPRNPA_Output& out1, const IPRNPA_Output& out2);
public:
  IPRNPA_Output(const IPRNPA_Input& i);
  IPRNPA_Output& operator=(const IPRNPA_Output& out);
  void Dump(ostream& os) const;
  void Reset();
  void PrettyPrint(ostream& os,string info) const;
  int RoomAssignment(unsigned p,unsigned d) const {return room_assignment[p][d];}
  int NurseAssignment(unsigned p,unsigned s) const {return nurse_assignment[p][s];}
  void SetRoom(unsigned p,unsigned d,int r){ room_assignment[p][d]=r;}
  void SetShift(unsigned p,unsigned s,int n){ nurse_assignment[p][s]=n;}
  int GetOccupant(unsigned r,unsigned d,unsigned i) const {return room_occupancy[r][d][i];}
  unsigned Occupancy(int r,unsigned d) const {return room_occupancy[r][d].size();}
  unsigned GetTransfers(unsigned p) const {return transfers[p];}
  void AddTransfer(unsigned p) {transfers[p]++;}
  void RemoveTransfer(unsigned p) {transfers[p]--;}
  void FindNewUB(unsigned d,unsigned r);
  void FindNewLB(unsigned d,unsigned r);

  void AddPatient(int p,unsigned i,unsigned r){room_occupancy[r][i].push_back(p);}
  void ErasePatient(unsigned p,unsigned d);
  int FindPatient(unsigned n,unsigned s,unsigned p);
  unsigned NumPatients(unsigned n,unsigned s) const{return nurse_list_patients[n][s].size();}
  int ExtractPatient(unsigned n,unsigned s,unsigned i) const{return nurse_list_patients[n][s][i];}

  float Load(unsigned n,unsigned s) const {return load[n][s];}
  float TotalLoad(unsigned n) const {return total_load[n];}
  unsigned EverAssigned(unsigned p,unsigned n) const {return ever_assigned[p][n];}
  unsigned NurseinRoom(unsigned n,unsigned s,unsigned r) const {return nurse_list_rooms[n][s][r];}

  unsigned NumFemales(unsigned r,unsigned d) const {return females[r][d];}
  int AgeMin(unsigned i,unsigned j)const{return age_occupancy[i][j].first.first;}
  int NumMin(unsigned r,unsigned d)const{return age_occupancy[r][d].first.second;}
  int AgeMax(unsigned r,unsigned d)const{return age_occupancy[r][d].second.first;}
  int NumMax(unsigned r,unsigned d)const{return age_occupancy[r][d].second.second;}
  void UpdateNurseData(int p,unsigned i,unsigned n);
  void UpdateRoomData(int p,unsigned i);
  void ComputeTotalLoad();

  void ChangeNurse(unsigned p,unsigned s,unsigned nn);
  void ChangeRoom(unsigned p,unsigned d,unsigned nr);
  void SwapNurses(unsigned s,unsigned p1,unsigned p2);
  void SwapRooms(unsigned d,unsigned p1,unsigned p2);

private:
  const IPRNPA_Input& in;
  vector<vector<int>> room_assignment; 
  vector<vector<int>> nurse_assignment;
  vector<vector<vector<int>>> room_occupancy;
  //Redundant nurse data
  vector<vector<vector<int>>> nurse_list_patients;
  vector<vector<vector<unsigned>>> nurse_list_rooms;
  vector<vector<float> >load;
  vector<double> total_load;
  vector<vector<unsigned>>nurse_ncov_rooms;
  vector<vector<int>>ever_assigned;
  vector<unsigned>patient_num_nurses;
  //Redundant patient data
  vector<unsigned>transfers;
  //Redundant Room data
  vector<vector<pair<pair<int,int>,pair<int,int>>>>age_occupancy;
  vector<vector<unsigned>> females;
};
#endif
