// File IPRNPA_Output.cc
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include "json.hpp"
#include "IPRNPA_Output.hh"

IPRNPA_Output::IPRNPA_Output(const IPRNPA_Input& my_in)
  : in(my_in),room_assignment(in.Patients(),vector<int>(in.Days(),-1)),
  	nurse_assignment(in.Patients(),vector<int>(in.Shifts(),-1)),
	room_occupancy(in.Rooms(),vector<vector<int>>(in.Days(),vector<int>())),
	transfers(in.Patients(),0),
	load(in.Nurses(),vector<float>(0)),
	total_load(in.Nurses(),0),
	ever_assigned(in.Patients(),vector<int>(in.Nurses(),0)),
	age_occupancy(in.Rooms(),vector<pair<pair<int,int>,pair<int,int>>>(in.Days(),make_pair(make_pair(-1,-1),make_pair(-1,-1)))),
	females(in.Rooms(),vector<unsigned>(in.Days(),0)),
	nurse_list_rooms(in.Nurses(),vector<vector<unsigned>>()),
	nurse_list_patients(in.Nurses(),vector<vector<int>>()),
	patient_num_nurses(in.Patients(),0),
	nurse_ncov_rooms(in.Nurses(),vector<unsigned>(0))
{
	unsigned ws;
	for(unsigned i=0;i<in.Nurses();i++){
		ws=in.GetNurse(i).working_shifts.size();
		load[i].resize(ws,0);
		nurse_ncov_rooms[i].resize(ws,0);
		nurse_list_patients[i].resize(ws);
		nurse_list_rooms[i].resize(ws,vector<unsigned>(in.Rooms(),0));
	}
}

void IPRNPA_Output::Dump(ostream& os) const
{
	const int pat_width=5;
	const int quad_width=24;
	unsigned i,j,k;
	string sep,pat,quad,day;
	os<<setw(pat_width);
	for(i=0;i<in.Days();i++){
		day=" Day "+to_string(i+1);
		os<<setw(quad_width-day.size())<<day;
	}
	os<<endl;
	for(i=0;i<in.Patients();i++){
		pat="P"+to_string(i+1);
		os<<setw(pat.size())<<pat;
		for(j=0;j<in.Days();j++){
			quad="(r"+to_string(room_assignment[i][j]+1)+",";
			for(k=0;k<3;k++){
				sep=k==2?"":",";
				quad=quad+to_string(nurse_assignment[i][(j*3)+k]+in.IndexOffset())+sep;
			}
			quad=quad+")";
			os<<setw(quad.size())<<quad;
		}
		os<<endl;
	}
	for(i=0;i<in.Nurses();i++){
		os<<"Printing loads of nurse "<<i<<":";
		for(j=0;j<in.GetNurse(i).working_shifts.size();j++)
			os<<load[i][j]<<" ";
		os<<"= "<<total_load[i]<<endl;
	}
	for(i=0;i<in.Rooms();i++){
		os<<"Room "<<i+1<<" occupancy: ";
		for(j=0;j<in.Days();j++){
			os<<"d"<<j<<"(";
			for(k=0;k<Occupancy(i,j);k++){
				os<<GetOccupant(i,j,k)<<", ";
			}
			os<<") ";
		}
		os<<endl;
	}
	for(i=0;i<in.Rooms();i++){
		os<<"Room "<<i+1<<"age occupancy: ";
		for(j=0;j<in.Days();j++)
			os<<"d"<<j<<" ("<<age_occupancy[i][j].first.first<<"--"<<age_occupancy[i][j].second.first<<") ";
	}
}

void IPRNPA_Output::Reset(){
	unsigned p,r,d,n,s;
	for(r=0;r<in.Rooms();r++){
		for(d=0;d<in.Days();d++){
			room_occupancy[r][d].clear();
			room_assignment[r][d]=-1;
			females[r][d]=0;
			age_occupancy[r][d]=make_pair(make_pair(-1,-1),make_pair(-1,-1));
		}
	}
	for(p=0;p<in.Patients();p++){
		transfers[p]=0;
		patient_num_nurses[p]=0;
		for(r=0;r<in.Nurses();r++)
			ever_assigned[p][r]=0;
		int admission=(in.GetPatient(p).admission-1),discharge=in.GetPatient(p).discharge;
		for(s=admission;s<discharge;s++){
			nurse_assignment[p][s]=-1;
		}
	}
	for(n=0;n<in.Nurses();n++){
		for(s=0;s<in.GetNurse(n).working_shifts.size();s++){
			load[n][s]=0;
			nurse_ncov_rooms[n][s]=0;
			nurse_list_patients[n][s].clear();
			for(r=0;r<in.Rooms();r++)
				nurse_list_rooms[n][s][r]=false;
		}
		total_load[n]=0;
	}
}

void IPRNPA_Output::PrettyPrint(ostream& os,string info) const
{
	int i,r,j,patient;
	nlohmann::json schedule;
	for(i=0;i<in.Shifts();i++){
		for(r=0;r<in.Rooms();r++){
			for (j=0;j<room_occupancy[r][i/3].size();j++){
				patient=room_occupancy[r][i/3][j];
				schedule[to_string(i+1)][to_string(r+1)][to_string(patient+1)]=to_string(NurseAssignment(patient,i)+in.IndexOffset());
			}
		}
	}
	schedule["_comments"] = info;
    os<< setw(4) << schedule << endl;
}

IPRNPA_Output& IPRNPA_Output::operator=(const IPRNPA_Output& out)
{
	room_assignment=out.room_assignment;
	nurse_assignment=out.nurse_assignment;
	room_occupancy=out.room_occupancy;
	transfers=out.transfers;
	load=out.load;
	total_load=out.total_load;
	nurse_list_patients=out.nurse_list_patients;
	ever_assigned=out.ever_assigned;
	patient_num_nurses=out.patient_num_nurses;
	nurse_list_rooms=out.nurse_list_rooms;
	nurse_ncov_rooms=out.nurse_ncov_rooms;
	age_occupancy=out.age_occupancy;
	females=out.females;
  	return *this;
}

void IPRNPA_Output::UpdateNurseData(int p,unsigned s,unsigned n){
	int ws=in.SearchWs(n,s);
	if (ws==-1){
		cerr<<"Nurse "<<n<<" shift"<<s<<endl;
		throw invalid_argument("shift index not found");
	}
	int ss=in.GetPatient(p).admission-1;
	nurse_list_patients[n][ws].push_back(p);
	load[n][ws]+=in.GetPatient(p).workload_req[s-ss]; 
	if(nurse_list_rooms[n][ws][room_assignment[p][s/3]]==0){
		nurse_ncov_rooms[n][ws]++;
	
	}
	nurse_list_rooms[n][ws][room_assignment[p][s/3]]++;
	if(ever_assigned[p][n]==0){
		patient_num_nurses[p]++;
	}
	ever_assigned[p][n]++;
}

void IPRNPA_Output::ComputeTotalLoad(){
	for(unsigned i=0;i<in.Nurses();i++){
		for(unsigned j=0;j<in.GetNurse(i).working_shifts.size();j++)
			total_load[i]+=load[i][j]/in.GetNurse(i).max_loads[j];
	}
}

void IPRNPA_Output::UpdateRoomData(int p,unsigned d){
 int room=room_assignment[p][d];
 int age=in.GetPatient(p).agegroup;
 if(age_occupancy[room][d].first.first==-1){ 
	age_occupancy[room][d].first.first=age;
	age_occupancy[room][d].first.second=1;
	age_occupancy[room][d].second.first=age;
	age_occupancy[room][d].second.second=1;
 }
 else if(age_occupancy[room][d].first.first>age){
	age_occupancy[room][d].first.first=age;
	age_occupancy[room][d].first.second=1;	
 }
 else if(age_occupancy[room][d].second.first<age){
	age_occupancy[room][d].second.first=age;
	age_occupancy[room][d].second.second=1;
 }
 else if(age_occupancy[room][d].first.first==age){
	age_occupancy[room][d].first.second++;
	if(age_occupancy[room][d].second.first==age)
		age_occupancy[room][d].second.second++;
 }
 else if(age_occupancy[room][d].second.first==age)
	age_occupancy[room][d].second.second++;
 if(in.GetPatient(p).gender==Gender::female)
	females[room][d]++;
}

/**
 @returns index of patient p inside the list of nurse n on shift s (where s is the shift relative index wrt nurse n)
 */
int IPRNPA_Output::FindPatient(unsigned n,unsigned s,unsigned p){
	for(unsigned i=0;i<nurse_list_patients[n][s].size();i++){
		if(nurse_list_patients[n][s][i]==p)
			return i;
	}
	return -1;
}

void IPRNPA_Output::ChangeNurse(unsigned p,unsigned s,unsigned nn){
	unsigned on=nurse_assignment[p][s];
	int ows=in.SearchWs(on,s);
	int nws=in.SearchWs(nn,s);
	float load_p=in.GetPatient(p).workload_req[s-(in.GetPatient(p).admission-1)];
	int day=s/3;
	load[on][ows]-=load_p;
	load[nn][nws]+=load_p;
	total_load[on]-=load_p/in.GetNurse(on).max_loads[ows];
	total_load[nn]+=load_p/in.GetNurse(nn).max_loads[nws];
	ever_assigned[p][on]--;
	ever_assigned[p][nn]++;
	if(ever_assigned[p][on]==0)
		patient_num_nurses[p]--;
	if(ever_assigned[p][nn]==1)
		patient_num_nurses[p]++;
	if(nurse_list_rooms[on][ows][room_assignment[p][day]]==1)
		nurse_ncov_rooms[on][ows]--;
	nurse_list_rooms[on][ows][room_assignment[p][day]]--;
	if(nurse_list_rooms[nn][nws][room_assignment[p][day]]==0)
		nurse_ncov_rooms[nn][nws]++;
	nurse_list_rooms[nn][nws][room_assignment[p][day]]++;
	nurse_list_patients[on][ows].erase(nurse_list_patients[on][ows].begin()+FindPatient(on,ows,p));
	nurse_list_patients[nn][nws].push_back(p);
	nurse_assignment[p][s]=nn;
}

void IPRNPA_Output::SwapNurses(unsigned s,unsigned p1,unsigned p2){
	int n1=NurseAssignment(p1,s),n2=NurseAssignment(p2,s),day=s/3;
	if(n1==-1 || n2==-2)
		throw invalid_argument("one of the patient is not assigned to anyone");
	int n1s=in.SearchWs(n1,s);
	int n2s=in.SearchWs(n2,s);
	float load_p1=in.GetPatient(p1).workload_req[s-(in.GetPatient(p1).admission-1)];
	float load_p2=in.GetPatient(p2).workload_req[s-(in.GetPatient(p2).admission-1)];
	load[n1][n1s]-=load_p1-load_p2;
	load[n2][n2s]+=load_p1-load_p2;
	total_load[n1]-=(load_p1-load_p2)/in.GetNurse(n1).max_loads[n1s];
	total_load[n2]+=(load_p1-load_p2)/in.GetNurse(n2).max_loads[n2s];
	ever_assigned[p1][n1]--;
	ever_assigned[p1][n2]++;
	ever_assigned[p2][n1]++;
	ever_assigned[p2][n2]--;
	if(ever_assigned[p1][n1]==0)
		patient_num_nurses[p1]--;
	if(ever_assigned[p1][n2]==1)
		patient_num_nurses[p1]++;
	if(ever_assigned[p2][n2]==0)
		patient_num_nurses[p2]--;
	if(ever_assigned[p2][n1]==1)
		patient_num_nurses[p2]++;
	//n1
	if(nurse_list_rooms[n1][n1s][room_assignment[p1][day]]==1)
		nurse_ncov_rooms[n1][n1s]--;
	nurse_list_rooms[n1][n1s][room_assignment[p1][day]]--;
	if(nurse_list_rooms[n1][n1s][room_assignment[p2][day]]==0)
		nurse_ncov_rooms[n1][n1s]++;
	nurse_list_rooms[n1][n1s][room_assignment[p2][day]]++;
	//n2
	if(nurse_list_rooms[n2][n2s][room_assignment[p2][day]]==1)
		nurse_ncov_rooms[n2][n2s]--;
	nurse_list_rooms[n2][n2s][room_assignment[p2][day]]--;
	if(nurse_list_rooms[n2][n2s][room_assignment[p1][day]]==0)
		nurse_ncov_rooms[n2][n2s]++;
	nurse_list_rooms[n2][n2s][room_assignment[p1][day]]++;
	
	nurse_list_patients[n1][n1s][FindPatient(n1,n1s,p1)]=p2;
	nurse_list_patients[n2][n2s][FindPatient(n2,n2s,p2)]=p1;
	nurse_assignment[p2][s]=n1;
	nurse_assignment[p1][s]=n2;
}

/**
 @remarks Nurse move rooms together with patient, so no changes to loads/skills compatibility
*/
void IPRNPA_Output::SwapRooms(unsigned d,unsigned p1,unsigned p2){ 
	int r1=RoomAssignment(p1,d),r2=RoomAssignment(p2,d),i,e_shift=d*3;
	if(d>0){
		if(r1!=RoomAssignment(p1,d-1) && r2==RoomAssignment(p1,d-1))
			transfers[p1]--;
		else if(r1==RoomAssignment(p1,d-1) && r2!=RoomAssignment(p1,d-1))
			transfers[p1]++;
		if(r2!=RoomAssignment(p2,d-1) && r1==RoomAssignment(p2,d-1))
			transfers[p2]--;
		else if(r2==RoomAssignment(p2,d-1) && r1!=RoomAssignment(p2,d-1))
			transfers[p2]++;
	}
	if(d<in.Days()-1){
		if(r1!=RoomAssignment(p1,d+1) && r2==RoomAssignment(p1,d+1))
			transfers[p1]--;
		else if(r1==RoomAssignment(p1,d+1) && r2!=RoomAssignment(p1,d+1))
			transfers[p1]++;
		if(r2!=RoomAssignment(p2,d+1) && r1==RoomAssignment(p2,d+1))
			transfers[p2]--;
		else if(r2==RoomAssignment(p2,d+1) && r1!=RoomAssignment(p2,d+1))
			transfers[p2]++;
	}
	for(i=0;i<Occupancy(r1,d);i++){
		if(GetOccupant(r1,d,i)==p1)
			room_occupancy[r1][d][i]=p2;
	}
	for(i=0;i<Occupancy(r2,d);i++){
		if(GetOccupant(r2,d,i)==p2)
			room_occupancy[r2][d][i]=p1;
	}
	if(in.GetPatient(p1).gender!=in.GetPatient(p2).gender){ 
		if(in.GetPatient(p1).gender==Gender::female){
			females[r1][d]--;
			females[r2][d]++;
		}
		else{
			females[r1][d]++;
			females[r2][d]--;
		}
	}
	int n1,n2,ws_n1,ws_n2;
	for(i=0;i<3;i++){
		n1=nurse_assignment[p1][e_shift+i],n2=nurse_assignment[p2][e_shift+i];
		ws_n1=in.SearchWs(n1,e_shift+i),ws_n2=in.SearchWs(n2,e_shift+i);
		if(n1!=n2){
			if(nurse_list_rooms[n1][ws_n1][room_assignment[p1][d]]==1)
				nurse_ncov_rooms[n1][ws_n1]--;
			nurse_list_rooms[n1][ws_n1][room_assignment[p1][d]]--;
			if(nurse_list_rooms[n1][ws_n1][room_assignment[p2][d]]==0)
				nurse_ncov_rooms[n1][ws_n1]++;
			nurse_list_rooms[n1][ws_n1][room_assignment[p2][d]]++;
			if(nurse_list_rooms[n2][ws_n2][room_assignment[p2][d]]==1)
				nurse_ncov_rooms[n2][ws_n2]--;
			nurse_list_rooms[n2][ws_n2][room_assignment[p2][d]]--;
			if(nurse_list_rooms[n2][ws_n2][room_assignment[p1][d]]==0)
				nurse_ncov_rooms[n2][ws_n2]++;
			nurse_list_rooms[n2][ws_n2][room_assignment[p1][d]]++;
		}
	}
	SetRoom(p1,d,r2);
	SetRoom(p2,d,r1);
	int age_1=in.GetPatient(p1).agegroup,age_2=in.GetPatient(p2).agegroup;
	if(age_1!=age_2){
		FindNewLB(d,r1);
		FindNewUB(d,r1);
		FindNewLB(d,r2);
		FindNewUB(d,r2);
	}
}


void IPRNPA_Output::FindNewLB(unsigned d,unsigned r){
	int lb=-1,age,n_min=1;
	for(unsigned i=0;i<room_occupancy[r][d].size();i++){
		age=in.GetPatient(room_occupancy[r][d][i]).agegroup;
		if(lb==-1)
			lb=age;
		else if(lb>age){
			lb=age;
			n_min=1;
		}
		else if(lb==age)
			n_min++;
	}
	if(lb==-1)
		age_occupancy[r][d].first=make_pair(-1,-1);
	else{
		age_occupancy[r][d].first.first=lb;
		age_occupancy[r][d].first.second=n_min;
	}
}

void IPRNPA_Output::FindNewUB(unsigned d,unsigned r){
	int ub=-1,age,n_max=1;
	for(unsigned i=0;i<room_occupancy[r][d].size();i++){
		age=in.GetPatient(room_occupancy[r][d][i]).agegroup;
		if(ub==-1)
			ub=age;
		else if(ub<age){
			ub=age;
			n_max=1;
		}
		else if(ub==age)
			n_max++;
	}
	if(ub==-1)
		age_occupancy[r][d].second=make_pair(-1,-1);
	else{
		age_occupancy[r][d].second.first=ub;
		age_occupancy[r][d].second.second=n_max;
	};
}

void IPRNPA_Output::ErasePatient(unsigned p,unsigned d){
	int room=room_assignment[p][d];
	for(unsigned i=0;i<room_occupancy[room][d].size();i++){
		if(room_occupancy[room][d][i]==p){
			room_occupancy[room][d].erase(room_occupancy[room][d].begin()+i);
			break;
		}
	}
}

void IPRNPA_Output::ChangeRoom(unsigned p,unsigned d,unsigned newr){
	int oldr=room_assignment[p][d];
	ErasePatient(p,d);
	room_assignment[p][d]=newr;	
	room_occupancy[newr][d].push_back(p);
	if(d>0 && oldr==room_assignment[p][d-1])
		transfers[p]++;
	if(d<in.Days()-1 && oldr==room_assignment[p][d+1])
		transfers[p]++;
	if(d>0 && newr==room_assignment[p][d-1])
		transfers[p]--;
	if(d<in.Days()-1 && newr==room_assignment[p][d+1])
		transfers[p]--;
	int age=in.GetPatient(p).agegroup;
	//Remove patient from oldr
	if(age_occupancy[oldr][d].first.first==age){
		if(age_occupancy[oldr][d].first.second>1)
			age_occupancy[oldr][d].first.second--;
		else	
			FindNewLB(d,oldr);
	}
	if(age_occupancy[oldr][d].second.first==age){
		if(age_occupancy[oldr][d].second.second>1)
			age_occupancy[oldr][d].second.second--;
		else	
			FindNewUB(d,oldr);
	}
	//Add patient to room newr
	if(age_occupancy[newr][d].first.first==-1){ 
		age_occupancy[newr][d].first.first=age;
		age_occupancy[newr][d].first.second=1;
		age_occupancy[newr][d].second.first=age;
		age_occupancy[newr][d].second.second=1;
	}
	else if(age_occupancy[newr][d].first.first>=age){
		if(age_occupancy[newr][d].first.first>age){
			age_occupancy[newr][d].first.first=age;
			age_occupancy[newr][d].first.second=1;
		}
		else
			age_occupancy[newr][d].first.second++;
	}
	else if(age_occupancy[newr][d].second.first<=age){
		if(age_occupancy[newr][d].second.first<age){
			age_occupancy[newr][d].second.first=age;
			age_occupancy[newr][d].second.second=1;
		}
		else
			age_occupancy[newr][d].second.second++;
	}
	if(in.GetPatient(p).gender==Gender::female){
		females[oldr][d]--;
		females[newr][d]++;
	}
	int s,n;
	for(unsigned i=0;i<3;i++){ 
		n=nurse_assignment[p][(d*3)+i];
		s=in.SearchWs(n,(d*3)+i);
		if(nurse_list_rooms[n][s][oldr]==1)
			nurse_ncov_rooms[n][s]--;
		if(nurse_list_rooms[n][s][newr]==0)
			nurse_ncov_rooms[n][s]++;
		nurse_list_rooms[n][s][oldr]--;
		nurse_list_rooms[n][s][newr]++;
	}
}
bool operator==(const IPRNPA_Output& out1, const IPRNPA_Output& out2)
{
	return true;
}

ostream& operator<<(ostream& os, const IPRNPA_Output& out)
{
	unsigned i,j,k;
	string sep,pat,quad,day;
	os<<endl;
	for(i=0;i<out.in.Patients();i++){
		pat="P"+to_string(i+1);
		os<<setw(pat.size())<<pat;
		for(j=0;j<out.in.Days();j++){
			if(out.RoomAssignment(i,j)!=-1){
				quad=" (r"+to_string(out.RoomAssignment(i,j)+1)+","+to_string(j+1)+",<";
				for(k=0;k<3;k++){
					sep=k==2?"":",";
					quad=quad+to_string(out.NurseAssignment(i,(j*3)+k)+out.in.IndexOffset())+sep; 
				}
				quad=quad+">)";
				os<<setw(quad.size())<<quad;
			}
		}
		os<<endl;
	}
	return os;
}
/**
 @remarks the assumption when reading the json is that the shifts are ordered as int, not lexicographic ordering. If needed use python script order_json ib python-scripts folder
*/
istream& operator>>(istream& is, IPRNPA_Output& out)
{
	nlohmann::json jsonData;
    is >> jsonData;
	int shift,room,patient,nurse;
	out.Reset();
	for (const auto& shiftEntry : jsonData.items()) {
		if (shiftEntry.key() == "_comments") 
            continue;
        shift = stoi(shiftEntry.key())-1;
        for (const auto& roomEntry : shiftEntry.value().items()) {
            room = stoi(roomEntry.key())-1;
            for (const auto& patientEntry : roomEntry.value().items()) {
                patient = stoi(patientEntry.key())-1;
                if (patientEntry.value().is_string()) 
                    nurse = stoi(patientEntry.value().get<std::string>());
				else
					nurse = patientEntry.value();
				if(shift%3==0){
					out.SetRoom(patient,shift/3,room);
					out.UpdateRoomData(patient,shift/3);
					if(shift/3>0 && out.RoomAssignment(patient,(shift/3)-1)!=-1 && out.RoomAssignment(patient,shift/3)!=out.RoomAssignment(patient,(shift/3)-1))
						out.AddTransfer(patient);
					out.AddPatient(patient,shift/3,room);
				}
                out.SetShift(patient,shift,nurse-out.in.IndexOffset());
				out.UpdateNurseData(patient,shift,nurse-out.in.IndexOffset());
            }
        }
    }
	out.ComputeTotalLoad();
	return is;
}