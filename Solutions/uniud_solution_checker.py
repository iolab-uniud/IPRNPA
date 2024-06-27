import json
import os
import sys
"""
github: https://github.com/TLKT0M/PRA_solution_check
"""
#BE CAREFUL: output json NEEDS TO be ordered (you can do it with order_json.py) before checking otherwise results will be erroneous
#Comment:added validation path to be declared for our convenience
class SolutionChecker:
    def __init__(self, instance_file, sol_file,valid_path='checked',weighted_obj_weights={    
                        #Comment: weights changed as described in the paper          
                        "transfers":11,
                        "inconvenience":1,
                        "gender_mix":5,
                        "equipment_vio":5,
                        "continuity_care":1,
                        "collected_nurse_vios": 5,
                        "min_nurse_num":2,
                        "walking_dist":0.05},
                        math_mode = True,three_shift=False):
        with open(instance_file, "r") as file:
            self.ins = json.load(file)
        with open(sol_file, "r") as file:
            self.data = json.load(file)
        #Comment: our addition to read correctly our output file
        if "_comments" in self.data:
            del self.data["_comments"]
        self.three_shift = three_shift
        self.init_instance()
        self.max_p,self.max_r = len(self.patients),len(self.rooms)
        if "days" in self.ins and "firstDay" in self.ins["days"] and "lastDay" in self.ins["days"]:
            self.max_s = (self.ins["days"]["lastDay"] - (self.ins["days"]["firstDay"] - 1)) * 3
        else:
            self.max_s = len(self.ins["shifts"])
        if math_mode:
            self.convert_to_readable()
        #Comment:changed added otherwise error with math_mode=False
        else:
            self.sol=self.data
        self.errors, self.violations, self.transfers= [],[],[]
        self.obj1= self.obj2= self.obj3= self.obj4= self.obj5= self.obj6 = 0
        self.obj7 =self.obj11 =self.vio_skill =self.vio_load =self.vio_fair = 0
        self.vio_fair_shift = 0

        self.check_room_capacity()
        self.check_gender_mixing()
        self.check_max_workload()
        self.check_patients_have_beds()
        self.check_skills_observed()
        self.calc_transfer_penalties()
        self.check_equipment_violations()
        
        self.check_age_violations()
        self.check_different_nurse_count()
        self.check_nurse_room_violations()
        self.check_fairness()
        self.check_fairness_overall()
        self.check_all_violations()
        self.check_distances()

        if not three_shift:
            self.obj3 = int(self.obj3 / 3)
            self.obj2 = int(self.obj2 / 3)
        self.obj11 = self.walkdistance
        #Comment: changed to map the weights into the objectives as stated in the objective_dict
        self.total = (
            weighted_obj_weights["transfers"] * self.obj1
            + weighted_obj_weights["inconvenience"] * self.obj2
            + weighted_obj_weights["gender_mix"] * self.obj3
            + weighted_obj_weights["equipment_vio"] * self.obj4
            + weighted_obj_weights["continuity_care"] * self.obj5
            + weighted_obj_weights["collected_nurse_vios"] * self.obj7
            + weighted_obj_weights["min_nurse_num"] * self.obj6
            + weighted_obj_weights["walking_dist"] * self.obj11
        )
        objective_dict = {"weighted_total_obj":self.total,
                        "unweighted_transfers_obj":self.obj1,
                        "unweighted_inconvenience_obj":self.obj2,
                        "unweighted_gender_mix_obj":self.obj3,
                        "unweighted_equipment_vio_obj":self.obj4,
                        "unweighted_continuity_care_obj":self.obj5,
                        "unweighted_collected_nurse_vios": self.obj7,
                        "unweighted_min_nurse_num_obj":self.obj6,
                        "unweighted_walking_dist_obj":self.obj11,
                        "unweighted_nurse_vio_skill_obj":self.vio_skill,
                        "unweighted_nurse_vio_load_obj":self.vio_load,
                        "unweighted_nurse_vio_fair_obj":self.vio_fair,
                        "unweighted_nurse_vio_fair_shift_obj":self.vio_fair_shift}
        #Comment:changed for our convenience
        """print(json.dumps(objective_dict,
              indent=4))"""
        #Comment:changed for our convenience
        os.makedirs(
            valid_path,
            exist_ok=True,
        )
        #Comment:changed for our convenience
        filename = os.path.join(valid_path,os.path.splitext(os.path.basename(instance_file))[0]+"_checked.json")
        with open(filename, "w") as f:
            json.dump(
                {
                    "objectives": objective_dict,
                    "errors": self.errors,
                    "violations": self.violations,
                    "transfers": self.transfers,             
                },
                f,
                indent=4,
            )

    def init_instance(self):
        rooms = {}
        for room in self.ins["rooms"]:
            rooms[str(room["id"])] = {
                "capacity": room["capacity"],
                "equipment": room["equipment"],
            }
        self.rooms = rooms
        patients = {}
        for patient in self.ins["patients"]:
            shifts = {}
            for shift in patient["workLoad"]:
                shifts[shift] = {
                    "skillReq": patient["skillReq"][shift],
                    "workload": patient["workLoad"][shift],
                    "equipmentReq": patient["equipmentReq"][shift],
                }
            patients[str(patient["id"])] = {
                "ageGroup": patient["ageGroup"],
                "admission": patient["admission"],
                "discharge": patient["discharge"],
                "gender": patient["gender"],
                "additional": shifts,
            }
        self.patients = patients
        nurses = {}
        for nurse in self.ins["nurses"]:
            w_shifts = {}
            for w_shift in nurse["workingShifts"]:
                w_shifts[str(w_shift)] = nurse["maxLoad"][str(w_shift)]
            nurses[str(nurse["id"])] = {
                "skillLevel": nurse["skillLevel"],
                "workingShifts": w_shifts,
            }
        self.nurses = nurses

    def check_distances(self):
        self.in_room = {}
        for nurse in self.nurses:
            n_in_room = {}
            for shift in self.sol:
                r_p_shift = {}

                for room in self.sol[shift]:

                    for patient in self.sol[shift][room]:
                        if self.sol[shift][room][patient] == nurse:
                            r_p_shift[room] = 1
                        else:
                            if room not in r_p_shift:
                                r_p_shift[room] = 0
                            if r_p_shift[room] != 1:
                                r_p_shift[room] = 0
                n_in_room[shift] = r_p_shift
            self.in_room[nurse] = n_in_room
            
        self.both_room = {}
        for n in self.in_room:
            n_dict = {}
            for s in self.in_room[n]:
                s_dict = {}
                for r1 in self.in_room[n][s]:
                    r_dict = {}
                    for r2 in self.in_room[n][s]:
                        if self.in_room[n][s][r1] == 1 and self.in_room[n][s][r2] == 1:
                            r_dict[r2] = 1
                        else:
                            r_dict[r2] = 0
                    s_dict[r1] = r_dict
                n_dict[s] = s_dict
            self.both_room[n] = n_dict
            
        self.walkdistance = 0
        self.dist_shift = {}
        for s in self.ins["shifts"]:
            s_dict = {}
            for n in self.nurses:
                s_dict[n] = 0
                if n in self.both_room and s in self.both_room[n]:
                    for r1 in self.both_room[n][s]:
                        s_dict[n] += (
                            self.ins["shifts"][s]["starWeight"]
                            * self.ins["distances"][str(0)][r1]
                            * self.in_room[n][s][r1]
                        )
                        for r2 in self.both_room[n][s][r1]:
                            if r1 != r2:
                                s_dict[n] += (
                                    self.ins["shifts"][s]["circleWeight"]
                                    * 0.5
                                    * self.ins["distances"][r1][r2]
                                    * self.both_room[n][s][r1][r2]
                                )
                    
            self.dist_shift[s] = s_dict
        for s in self.dist_shift:
            for n in self.dist_shift[s]:
                self.walkdistance += self.dist_shift[s][n]

    def check_all_violations(self):
        self.obj7 = self.vio_fair + self.vio_fair_shift + self.vio_load +self.vio_skill

    def check_fairness_factor(self):
        for shift in self.sol:
            for room in self.sol[shift]:
                nurse_list = []
                for patient in self.sol[shift][room]:
                    if self.sol[shift][room][patient] not in nurse_list:
                        nurse_list.append(self.sol[shift][room][patient])
                self.obj7 += (len(nurse_list) - 1) * 2

    def check_nurse_room_violations(self):
        for shift in self.sol:
            for room in self.sol[shift]:
                nurse_list = []
                for patient in self.sol[shift][room]:
                    if self.sol[shift][room][patient] not in nurse_list:
                        nurse_list.append(self.sol[shift][room][patient])
                        self.obj6 += 1

    def check_different_nurse_count(self):
        for cur_patient in self.patients:
            self.patients[cur_patient]["nurse_list"] = []
            for shift in self.sol:
                for room in self.sol[shift]:
                    for patient in self.sol[shift][room]:
                        if cur_patient == patient:
                            if (
                                self.sol[shift][room][patient]
                                not in self.patients[cur_patient]["nurse_list"]
                            ):
                                self.patients[cur_patient]["nurse_list"].append(
                                    self.sol[shift][room][patient]
                                )

            self.obj5 += len(self.patients[cur_patient]["nurse_list"])

    def check_age_violations(self):
        for shift in self.sol:
            for room in self.sol[shift]:
                age_min = None
                age_max = None
                for patient in self.sol[shift][room]:
                    if age_min is None:                    
                        age_min = self.patients[patient]["ageGroup"]
                    else:
                        if age_min >= self.patients[patient]["ageGroup"]:
                            age_min = self.patients[patient]["ageGroup"]
                    if age_max is None:
                        age_max = self.patients[patient]["ageGroup"]
                    else:
                        if age_max <= self.patients[patient]["ageGroup"]:
                            age_max = self.patients[patient]["ageGroup"]
                if age_min !=None and age_max != None:
                    self.obj2 += age_max - age_min



    def check_equipment_violations(self):

        self.obj4 = 0

        for shift in self.sol:
            for room in self.sol[shift]:
                for patient in self.sol[shift][room]:
      
                    if (
                        self.patients[patient]["admission"]
                        <= int(shift)
                        <= self.patients[patient]["discharge"]
                    ):

                        required_equipment = self.patients[patient]["additional"][
                            shift
                        ]["equipmentReq"]
                        room_equipment = self.rooms[room]["equipment"]
                        if any(
                            equip not in room_equipment for equip in required_equipment
                        ):
                            if not self.three_shift:
                                if (int(shift) + 2) % 3 == 0:
                                    self.obj4 += 1
                                    violation_msg = f"EquipmentViolation: shift: {shift} patient: {patient} room: {room}"
                                    self.violations.append(violation_msg)
                            else:
                                self.obj4 += 1
                                violation_msg = f"EquipmentViolation: shift: {shift} patient: {patient} room: {room}"
                                self.violations.append(violation_msg)



    def calc_transfer_penalties(self):
        for cur_patient in self.patients:
            self.patients[cur_patient]["assigned_room"] = None
            for shift in self.sol:
                for room in self.sol[shift]:
                    for patient in self.sol[shift][room]:
                        if cur_patient == patient:
                            if self.patients[cur_patient]["assigned_room"] is None:
                                self.patients[cur_patient]["assigned_room"] = room
                            elif self.patients[cur_patient]["assigned_room"] != room:
                                violation_msg = f"TransferMessage: shift: {shift} patient: {patient} from room: {self.patients[cur_patient]['assigned_room']} to room: {room}"
                                self.transfers.append(violation_msg)
                                self.patients[cur_patient]["assigned_room"] = room
                                self.obj1 += 1

    def check_skills_observed(self):

        for shift in self.sol:
            for room in self.sol[shift]:
                for patient in self.sol[shift][room]:
                    nurse_skill = self.nurses[self.sol[shift][room][patient]][
                        "skillLevel"
                    ]
                    patient_skill_req = self.patients[patient]["additional"][
                        str(shift)
                    ]["skillReq"]
                    if nurse_skill < patient_skill_req:
                        if not self.three_shift:
                            if (int(shift) + 2) % 3 <=1:
                                violation_msg = f"NurseSkillLevelRequirementViolation: shift: {shift} nurse: {self.sol[shift][room][patient]} with skill level {nurse_skill} does not fulfill skill level {patient_skill_req} of patient {patient} in shift {shift}"
                                self.violations.append(violation_msg)
                                self.vio_skill += 1
                        else:
                            violation_msg = f"NurseSkillLevelRequirementViolation: shift: {shift} nurse: {self.sol[shift][room][patient]} with skill level {nurse_skill} does not fulfill skill level {patient_skill_req} of patient {patient} in shift {shift}"
                            self.violations.append(violation_msg)
                            self.vio_skill += 1


    def check_patients_have_beds(self):
        for shift in self.sol:
            cur_patients = []
            for patient in self.patients:
                if shift in self.patients[patient]["additional"]:
                    cur_patients.append(patient)

            for room in self.sol[shift]:
                for patient in self.sol[shift][room]:
                    if patient in cur_patients:
                        cur_patients.remove(patient)
            if len(cur_patients) > 0:
                for patient in cur_patients:
                    self.errors.append(
                        "PatientNoBedError: shift: "
                        + str(shift)
                        + " patient: "
                        + str(patient)
                    )

    def check_max_workload(self):
        for shift in self.sol:
            for nurse in self.nurses:
                cur_wl = 0
                for room in self.sol[shift]:
                    for patient in self.sol[shift][room]:
                        if self.sol[shift][room][patient] == nurse:
                            cur_wl += self.patients[patient]["additional"][str(shift)][
                                "workload"
                            ]

                if str(shift) in self.nurses[nurse]["workingShifts"]:
                    if self.nurses[nurse]["workingShifts"][str(shift)] < cur_wl:
                        violation_msg = f"WorkLoadViolationError: shift: {shift}  nurse: {nurse} has exceed workload {cur_wl}/{self.nurses[nurse]['workingShifts'][str(shift)]} "
                        self.violations.append(violation_msg)
                        self.vio_load += (
                            cur_wl - self.nurses[nurse]["workingShifts"][str(shift)]
                        )

    def check_gender_mixing(self):
        for shift in self.sol:
            for room in self.sol[shift]:
                gender = None
                gender_mixing = False
                patient_genders = []
                for patient in self.sol[shift][room]:
                    if gender is None:
                        gender = self.patients[patient]["gender"]
                        patient_genders.append(
                            {
                                "Patient "
                                + str(patient): self.patients[patient]["gender"]
                            }
                        )
                    else:
                        if gender_mixing == False:
                            if gender != self.patients[patient]["gender"]:
                                gender_mixing = True
                            
                                if not self.three_shift:
                                    if (int(shift) + 2) % 3 == 0:
                                        violation_msg = f"RoomGenderMixingViolation: shift: {shift} room: {room} has mixed genders. Current room gender: {gender} mixed with {self.patients[patient]['gender']} from patient: {patient}"
                                        self.violations.append(violation_msg)
                                else:
                                    violation_msg = f"RoomGenderMixingViolation: shift: {shift} room: {room} has mixed genders. Current room gender: {gender} mixed with {self.patients[patient]['gender']} from patient: {patient}"
                                    self.violations.append(violation_msg)
                                
                                self.obj3 += 1
                                patient_genders.append(
                                    {
                                        "Patient "
                                        + str(patient): self.patients[patient]["gender"]
                                    }
                                )

    def check_room_capacity(self):
        for shift in self.sol:
            for room in self.sol[shift]:
                if len(self.sol[shift][room]) > self.rooms[room]["capacity"]:
                    error_msg = f"RoomCapacityExceededError: shift: {shift} room: {room} with capacity {self.rooms[room]['capacity']} is exceeded by {len(self.sol[shift][room])} patients."
                    self.errors.append(error_msg)

    def check_fairness(self):
    
        self.vio_fair_shift = 0
        for shift in self.sol:
            nurse_workloads = {nurse: 0 for nurse in self.nurses}

            for room in self.sol[shift]:
                for patient, nurse in self.sol[shift][room].items():
                    if nurse in nurse_workloads:
                        nurse_workloads[nurse] += self.patients[patient]["additional"][
                            shift
                        ]["workload"]

 
            for n in nurse_workloads:
                for n_prime in nurse_workloads:
                    if (
                        n != n_prime
                        and self.nurses[n]["workingShifts"].get(shift, 0) > 0
                        and self.nurses[n_prime]["workingShifts"].get(shift, 0) > 0
                    ):

                        relative_workload_n = (
                            nurse_workloads[n] / self.nurses[n]["workingShifts"][shift]
                        )
                        relative_workload_n_prime = (
                            nurse_workloads[n_prime]
                            / self.nurses[n_prime]["workingShifts"][shift]
                        )
                        if relative_workload_n > relative_workload_n_prime:
                            self.vio_fair_shift += (
                                relative_workload_n - relative_workload_n_prime
                            )

    def calculate_relative_workload(self, nurse_id, shift):
        total_workload = 0
        max_workload = self.nurses[nurse_id]["workingShifts"].get(
                                                shift, float("inf"))
        for room in self.sol[shift]:
            for patient, nurse in self.sol[shift][room].items():
                if nurse == nurse_id:
                    total_workload += self.patients[patient][
                        "additional"
                        ][shift]["workload"]
        return total_workload / max_workload if max_workload else 0

    def check_fairness_overall(self):
        self.vio_fair = 0  
        for n in self.nurses:
            for n_prime in self.nurses:
                if n != n_prime:
                    relative_workload_n = sum(
                        self.calculate_relative_workload(n, shift)
                        for shift in self.nurses[n]["workingShifts"]
                    )
                    relative_workload_n_prime = sum(
                        self.calculate_relative_workload(n_prime, shift)
                        for shift in self.nurses[n_prime]["workingShifts"]
                    )

                    if relative_workload_n > relative_workload_n_prime:
                        self.vio_fair += (relative_workload_n -
                                          relative_workload_n_prime)

    def convert_to_readable(self):
        invert = {}
        for s in range(self.max_s):
            shift = {}
            for r in range(self.max_r):
                shift[str(r + 1)] = {}
            invert[str(s + 1)] = shift

        for p in self.data["y"]:
            for r in self.data["y"][p]:
                for s in self.data["y"][p][r]:
                    if self.data["y"][p][r][s] == 1:
                        invert[s][r][p] = self.get_nurse(p, s)
                        invert[str(int(s) + 1)][r][p] = self.get_nurse(
                            p, str(int(s) + 1)
                        )
                        invert[str(int(s) + 2)][r][p] = self.get_nurse(
                            p, str(int(s) + 2)
                        )
        self.sol = invert
        

    def get_nurse(self, p, s):
        for n in self.data["x"][p]:
            if s in self.data["x"][p][n]:
                if self.data["x"][p][n][s] == 1:
                    return n

# For Development example
#solutionchecker = SolutionChecker("./instance/instance.json", "./solutions/solution_math_mode_true.json")
#solutionchecker = SolutionChecker("../../../real_world_instances/UMD_instance_01.json", "./ordered_test_real.json",math_mode=False)
