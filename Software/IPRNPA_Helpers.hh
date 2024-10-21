// File IPRNPA_Helpers.hh
#ifndef IPRNPA_HELPERS_HH
#define IPRNPA_HELPERS_HH

#include "IPRNPA_Output.hh"
#include <easylocal.hh>

using namespace EasyLocal::Core;

typedef double cost_type;

/***************************************************************************
 * Solution Manager 
 ***************************************************************************/

class IPRNPA_SolutionManager : public SolutionManager<IPRNPA_Input,IPRNPA_Output,DefaultCostStructure<cost_type>> 
{
public:
  IPRNPA_SolutionManager(const IPRNPA_Input &);
  void RandomState(IPRNPA_Output& out);   
  void DumpState(const IPRNPA_Output& out, ostream& os) const;   
  bool CheckConsistency(const IPRNPA_Output& st)const;
  void PrettyPrintOutput(const IPRNPA_Output& st, string filename) const;
protected:
}; 

class IPRNPA_Transfer : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_Transfer(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Transfer") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class  IPRNPA_AgeGroup: public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_AgeGroup(const IPRNPA_Input & in, int w, bool hard) : CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_AgeGroup") 
  {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_Gender : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_Gender(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Gender") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_Equipment : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_Equipment(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Equipment") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_Continuity : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_Continuity(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Continuity") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_Skill : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_Skill(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Skill") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_Excess : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_Excess(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Excess") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_TotalBalance : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_TotalBalance(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Total") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_ShiftBalance : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_ShiftBalance(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_Load") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_NursesperRoom : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_NursesperRoom(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_NursesperRoom") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_WalkingCircle : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_WalkingCircle(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_WalkingCircle") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

class IPRNPA_WalkingStar : public CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type> 
{
public:
  IPRNPA_WalkingStar(const IPRNPA_Input & in, int w, bool hard) :    CostComponent<IPRNPA_Input,IPRNPA_Output,cost_type>(in,w,hard,"IPRNPA_WalkingStar") {}
  cost_type ComputeCost(const IPRNPA_Output& st) const override;
  void PrintViolations(const IPRNPA_Output& st, ostream& os = cout) const override;
};

/***************************************************************************
 * IPRNPA_Change Neighborhood Explorer:
 ***************************************************************************/

class IPRNPA_ChangeNurse
{
  friend bool operator==(const IPRNPA_ChangeNurse& m1, const IPRNPA_ChangeNurse& m2);
  friend bool operator!=(const IPRNPA_ChangeNurse& m1, const IPRNPA_ChangeNurse& m2);
  friend bool operator<(const IPRNPA_ChangeNurse& m1, const IPRNPA_ChangeNurse& m2);
  friend ostream& operator<<(ostream& os, const IPRNPA_ChangeNurse& mv);
  friend istream& operator>>(istream& is, IPRNPA_ChangeNurse& mv);
 public:
  unsigned patient,shift,old_nurse,new_nurse;
};

class IPRNPA_ChangeRoom
{
  friend bool operator==(const IPRNPA_ChangeRoom& m1, const IPRNPA_ChangeRoom& m2);
  friend bool operator!=(const IPRNPA_ChangeRoom& m1, const IPRNPA_ChangeRoom& m2);
  friend bool operator<(const IPRNPA_ChangeRoom& m1, const IPRNPA_ChangeRoom& m2);
  friend ostream& operator<<(ostream& os, const IPRNPA_ChangeRoom& mv);
  friend istream& operator>>(istream& is, IPRNPA_ChangeRoom& mv);
 public:
  unsigned patient,day,old_room,new_room;
};

class IPRNPA_SwapNurses
{
  friend bool operator==(const IPRNPA_SwapNurses& m1, const IPRNPA_SwapNurses& m2);
  friend bool operator!=(const IPRNPA_SwapNurses& m1, const IPRNPA_SwapNurses& m2);
  friend bool operator<(const IPRNPA_SwapNurses& m1, const IPRNPA_SwapNurses& m2);
  friend ostream& operator<<(ostream& os, const IPRNPA_SwapNurses& mv);
  friend istream& operator>>(istream& is, IPRNPA_SwapNurses& mv);
 public:
  unsigned p1,p2,shift,n1,n2;
  int p1_idx,p2_idx;
};

class IPRNPA_SwapRooms
{
  friend bool operator==(const IPRNPA_SwapRooms& m1, const IPRNPA_SwapRooms& m2);
  friend bool operator!=(const IPRNPA_SwapRooms& m1, const IPRNPA_SwapRooms& m2);
  friend bool operator<(const IPRNPA_SwapRooms& m1, const IPRNPA_SwapRooms& m2);
  friend ostream& operator<<(ostream& os, const IPRNPA_SwapRooms& mv);
  friend istream& operator>>(istream& is, IPRNPA_SwapRooms& mv);
 public:
  unsigned p1,p2,day,n11,n21,n12,n22,n13,n23;
  int room_1,room_2,p1_idx,p2_idx;
};

class IPRNPA_ChangeNurseNeighborhoodExplorer
  : public NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,DefaultCostStructure<cost_type>> 
{
public:
  IPRNPA_ChangeNurseNeighborhoodExplorer(const IPRNPA_Input & pin, SolutionManager<IPRNPA_Input,IPRNPA_Output,DefaultCostStructure<cost_type>>& psm)  
    : NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,DefaultCostStructure<cost_type>>(pin, psm, "IPRNPA_ChangeNurseNeighborhoodExplorer") {} 
  void RandomMove(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const override;          
  bool FeasibleMove(const IPRNPA_Output&, const IPRNPA_ChangeNurse&) const override;  
  void MakeMove(IPRNPA_Output&, const IPRNPA_ChangeNurse&) const override;             
  void FirstMove(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const override;  
  bool NextMove(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const override;   
protected:
  void AnyFirstMove(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const;  
  bool AnyNextMove(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const;  
  void FirstNurse(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const; 
  bool NextNurse(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const;
  bool NextShift(const IPRNPA_Output&, IPRNPA_ChangeNurse&) const;
};

class IPRNPA_ChangeRoomNeighborhoodExplorer
  : public NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,DefaultCostStructure<cost_type>> 
{
public:
  IPRNPA_ChangeRoomNeighborhoodExplorer(const IPRNPA_Input & pin, SolutionManager<IPRNPA_Input,IPRNPA_Output,DefaultCostStructure<cost_type>>& psm)  
    : NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,DefaultCostStructure<cost_type>>(pin, psm, "IPRNPA_ChangeRoomNeighborhoodExplorer") {} 
  void RandomMove(const IPRNPA_Output&, IPRNPA_ChangeRoom&) const override;          
  bool FeasibleMove(const IPRNPA_Output&, const IPRNPA_ChangeRoom&) const override;  
  void MakeMove(IPRNPA_Output&, const IPRNPA_ChangeRoom&) const override;             
  void FirstMove(const IPRNPA_Output&, IPRNPA_ChangeRoom&) const override;  
  bool NextMove(const IPRNPA_Output&, IPRNPA_ChangeRoom&) const override;   
protected:
  void FirstRoom(const IPRNPA_Output&, IPRNPA_ChangeRoom&) const; 
  bool NextRoom(const IPRNPA_Output&, IPRNPA_ChangeRoom&) const;
  bool NextDay(const IPRNPA_Output&, IPRNPA_ChangeRoom&) const;
};

class IPRNPA_SwapNursesNeighborhoodExplorer
  : public NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,DefaultCostStructure<cost_type>> 
{
public:
  IPRNPA_SwapNursesNeighborhoodExplorer(const IPRNPA_Input & pin, SolutionManager<IPRNPA_Input,IPRNPA_Output,DefaultCostStructure<cost_type>>& psm)  
    : NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,DefaultCostStructure<cost_type>>(pin, psm, "IPRNPA_SwapNursesNeighborhoodExplorer") {} 
  void RandomMove(const IPRNPA_Output&, IPRNPA_SwapNurses&) const override;          
  bool FeasibleMove(const IPRNPA_Output&, const IPRNPA_SwapNurses&) const override;  
  void MakeMove(IPRNPA_Output&, const IPRNPA_SwapNurses&) const override;             
  void FirstMove(const IPRNPA_Output&, IPRNPA_SwapNurses&) const override;  
  bool NextMove(const IPRNPA_Output&, IPRNPA_SwapNurses&) const override;   
protected:
  void AnyFirstMove(const IPRNPA_Output&, IPRNPA_SwapNurses&) const;  
  bool AnyNextMove(const IPRNPA_Output&, IPRNPA_SwapNurses&) const;  
  bool NextPatientOne(const IPRNPA_Output&, IPRNPA_SwapNurses&) const; 
  bool NextPatientTwo(const IPRNPA_Output&, IPRNPA_SwapNurses&) const; 
  bool NextShift(const IPRNPA_Output&, IPRNPA_SwapNurses&) const;
};


class IPRNPA_SwapRoomsNeighborhoodExplorer
  : public NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,DefaultCostStructure<cost_type>> 
{
public:
  IPRNPA_SwapRoomsNeighborhoodExplorer(const IPRNPA_Input & pin, SolutionManager<IPRNPA_Input,IPRNPA_Output,DefaultCostStructure<cost_type>>& psm)  
    : NeighborhoodExplorer<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,DefaultCostStructure<cost_type>>(pin, psm, "IPRNPA_SwapRoomsNeighborhoodExplorer") {} 
  void RandomMove(const IPRNPA_Output&, IPRNPA_SwapRooms&) const override;          
  bool FeasibleMove(const IPRNPA_Output&, const IPRNPA_SwapRooms&) const override;  
  void MakeMove(IPRNPA_Output&, const IPRNPA_SwapRooms&) const override;             
  void FirstMove(const IPRNPA_Output&, IPRNPA_SwapRooms&) const override;  
  bool NextMove(const IPRNPA_Output&, IPRNPA_SwapRooms&) const override;   
protected:
  void AnyFirstMove(const IPRNPA_Output&, IPRNPA_SwapRooms&) const;  
  bool AnyNextMove(const IPRNPA_Output&, IPRNPA_SwapRooms&) const;  
  bool NextPatientOne(const IPRNPA_Output&, IPRNPA_SwapRooms&) const; 
  bool NextPatientTwo(const IPRNPA_Output&, IPRNPA_SwapRooms&) const; 
  bool NextDay(const IPRNPA_Output&, IPRNPA_SwapRooms&) const;
  void FirstPatientPair(const IPRNPA_Output&, IPRNPA_SwapRooms&) const;
  void FirstPatientTwo(const IPRNPA_Output&, IPRNPA_SwapRooms&) const;
};

class IPRNPA_ChangeNurseDeltaContinuity
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaContinuity(const IPRNPA_Input & in, IPRNPA_Continuity& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaContinuity") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};

class IPRNPA_ChangeNurseDeltaSkill
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaSkill(const IPRNPA_Input & in, IPRNPA_Skill& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaSkill") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};

class IPRNPA_ChangeNurseDeltaExcess
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaExcess(const IPRNPA_Input & in, IPRNPA_Excess& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaExcess") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};

class IPRNPA_ChangeNurseDeltaTotalBalance
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaTotalBalance(const IPRNPA_Input & in, IPRNPA_TotalBalance& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaTotalBalance") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};

class IPRNPA_ChangeNurseDeltaShiftBalance
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaShiftBalance(const IPRNPA_Input & in, IPRNPA_ShiftBalance& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaShiftBalance") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};

class IPRNPA_ChangeNurseDeltaNursesperRoom
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaNursesperRoom(const IPRNPA_Input & in, IPRNPA_NursesperRoom& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaNursesperRoom") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};


class IPRNPA_ChangeNurseDeltaWalkingCircle
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaWalkingCircle(const IPRNPA_Input & in, IPRNPA_WalkingCircle& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaWalkingCircle") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};

class IPRNPA_ChangeNurseDeltaWalkingStar
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>
{
public:
  IPRNPA_ChangeNurseDeltaWalkingStar(const IPRNPA_Input & in, IPRNPA_WalkingStar& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeNurse,cost_type>(in,cc,"IPRNPA_ChangeNurseDeltaWalkingStar") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeNurse& mv) const;
};

class IPRNPA_ChangeRoomDeltaTransfer
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>
{
public:
  IPRNPA_ChangeRoomDeltaTransfer(const IPRNPA_Input & in, IPRNPA_Transfer& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>(in,cc,"IPRNPA_ChangeRoomDeltaTransfer") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeRoom& mv) const;
};

class IPRNPA_ChangeRoomDeltaAgeGroup
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>
{
public:
  IPRNPA_ChangeRoomDeltaAgeGroup(const IPRNPA_Input & in, IPRNPA_AgeGroup& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>(in,cc,"IPRNPA_ChangeRoomDeltaAgeGroup") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeRoom& mv) const;
};

class IPRNPA_ChangeRoomDeltaGender
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>
{
public:
  IPRNPA_ChangeRoomDeltaGender(const IPRNPA_Input & in, IPRNPA_Gender& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>(in,cc,"IPRNPA_ChangeRoomDeltaGender") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeRoom& mv) const;
};

class IPRNPA_ChangeRoomDeltaEquipment
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>
{
public:
  IPRNPA_ChangeRoomDeltaEquipment(const IPRNPA_Input & in, IPRNPA_Equipment& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>(in,cc,"IPRNPA_ChangeRoomDeltaEquipment") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeRoom& mv) const;
};

class IPRNPA_ChangeRoomDeltaNursesperRoom
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>
{
public:
  IPRNPA_ChangeRoomDeltaNursesperRoom(const IPRNPA_Input & in, IPRNPA_NursesperRoom& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>(in,cc,"IPRNPA_ChangeRoomDeltaNursesperRoom") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeRoom& mv) const;
};

class IPRNPA_ChangeRoomDeltaWalkingCircle
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>
{
public:
  IPRNPA_ChangeRoomDeltaWalkingCircle(const IPRNPA_Input & in, IPRNPA_WalkingCircle& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>(in,cc,"IPRNPA_ChangeRoomDeltaWalkingCircle") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeRoom& mv) const;
};

class IPRNPA_ChangeRoomDeltaWalkingStar
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>
{
public:
  IPRNPA_ChangeRoomDeltaWalkingStar(const IPRNPA_Input & in, IPRNPA_WalkingStar& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_ChangeRoom,cost_type>(in,cc,"IPRNPA_ChangeRoomDeltaWalkingStar") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_ChangeRoom& mv) const;
};

class IPRNPA_SwapNursesDeltaContinuity
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaContinuity(const IPRNPA_Input & in, IPRNPA_Continuity& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaContinuity") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapNursesDeltaSkill
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaSkill(const IPRNPA_Input & in, IPRNPA_Skill& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaSkill") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapNursesDeltaExcess
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaExcess(const IPRNPA_Input & in, IPRNPA_Excess& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaExcess") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapNursesDeltaTotalBalance
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaTotalBalance(const IPRNPA_Input & in, IPRNPA_TotalBalance& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaTotalBalance") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapNursesDeltaShiftBalance
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaShiftBalance(const IPRNPA_Input & in, IPRNPA_ShiftBalance& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaShiftBalance") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapNursesDeltaNursesperRoom
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaNursesperRoom(const IPRNPA_Input & in, IPRNPA_NursesperRoom& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaNursesperRoom") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapNursesDeltaWalkingCircle
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaWalkingCircle(const IPRNPA_Input & in, IPRNPA_WalkingCircle& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaWalkingCircle") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapNursesDeltaWalkingStar
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>
{
public:
  IPRNPA_SwapNursesDeltaWalkingStar(const IPRNPA_Input & in, IPRNPA_WalkingStar& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapNurses,cost_type>(in,cc,"IPRNPA_SwapNursesDeltaWalkingStar") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapNurses& mv) const;
};

class IPRNPA_SwapRoomsDeltaTransfer
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>
{
public:
  IPRNPA_SwapRoomsDeltaTransfer(const IPRNPA_Input & in, IPRNPA_Transfer& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>(in,cc,"IPRNPA_SwapRoomsDeltaTransfer") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapRooms& mv) const;
};

class IPRNPA_SwapRoomsDeltaAgeGroup
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>
{
public:
  IPRNPA_SwapRoomsDeltaAgeGroup(const IPRNPA_Input & in, IPRNPA_AgeGroup& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>(in,cc,"IPRNPA_SwapRoomsDeltaAgeGroup") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapRooms& mv) const;
};

class IPRNPA_SwapRoomsDeltaGender
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>
{
public:
  IPRNPA_SwapRoomsDeltaGender(const IPRNPA_Input & in, IPRNPA_Gender& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>(in,cc,"IPRNPA_SwapRoomsDeltaGender") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapRooms& mv) const;
};

class IPRNPA_SwapRoomsDeltaEquipment
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>
{
public:
  IPRNPA_SwapRoomsDeltaEquipment(const IPRNPA_Input & in, IPRNPA_Equipment& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>(in,cc,"IPRNPA_SwapRoomsDeltaEquipment") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapRooms& mv) const;
};

class IPRNPA_SwapRoomsDeltaNursesperRoom
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>
{
public:
  IPRNPA_SwapRoomsDeltaNursesperRoom(const IPRNPA_Input & in, IPRNPA_NursesperRoom& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>(in,cc,"IPRNPA_SwapRoomsDeltaNursesperRoom") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapRooms& mv) const;
};

class IPRNPA_SwapRoomsDeltaWalkingCircle
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>
{
public:
  IPRNPA_SwapRoomsDeltaWalkingCircle(const IPRNPA_Input & in, IPRNPA_WalkingCircle& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>(in,cc,"IPRNPA_SwapRoomsDeltaWalkingCircle") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapRooms& mv) const;
};

class IPRNPA_SwapRoomsDeltaWalkingStar
  : public DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>
{
public:
  IPRNPA_SwapRoomsDeltaWalkingStar(const IPRNPA_Input & in, IPRNPA_WalkingStar& cc) 
    : DeltaCostComponent<IPRNPA_Input,IPRNPA_Output,IPRNPA_SwapRooms,cost_type>(in,cc,"IPRNPA_SwapRoomsDeltaWalkingStar") {}
  cost_type ComputeDeltaCost(const IPRNPA_Output& st, const IPRNPA_SwapRooms& mv) const;
};



#endif
