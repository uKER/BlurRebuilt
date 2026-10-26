-- InGame Scoring Logic

ScoringEvent_Debug = 0



--	Amax.AddOnTrackScore(AmaxScoringEvent.VehicleIndex,"Hit with "..GetPerkName(AmaxScoringEvent.PerkUsed),1,false)
--	Amax.AddOnTrackScore(AmaxScoringEvent.VehicleIndex,"Wrecked",2,true)


function AddScoringEvent(type)

	if type == UIEnums.TrackScoring.ScoringType.UsedPerk then
		
		if ScoringEvent_Debug == 1 and AmaxScoringEvent.VehicleIndex == 0 then
			print("-- ",GetScoringEventName(type))
			print("Vehicle Index",AmaxScoringEvent.VehicleIndex)
			print("Perk Used",GetPerkName(AmaxScoringEvent.PerkUsed))
			print("Level",AmaxScoringEvent.Level)
			print("Hit Index",AmaxScoringEvent.VehicleHitIndex)
			print("Unique ID",AmaxScoringEvent.UniqueID)
			print("--")
			print("")
		end

	elseif type == UIEnums.TrackScoring.ScoringType.ShieldedPerk then

		if ScoringEvent_Debug == 1 and AmaxScoringEvent.VehicleIndex == 0 then
			print("-- ",GetScoringEventName(type))
			print("Vehicle Index",AmaxScoringEvent.VehicleIndex)
			print("DefendedFromVehicleIndex",AmaxScoringEvent.DefendedFromVehicleIndex)
			print("--")
			print("")
		end

	elseif type == UIEnums.TrackScoring.ScoringType.Overtake then
		
		if ScoringEvent_Debug == 1 and AmaxScoringEvent.VehicleIndex == 0 then
			print("-- ",GetScoringEventName(type))
			print("Vehicle Index",AmaxScoringEvent.VehicleIndex)
			print("NewRacePosition",AmaxScoringEvent.NewRacePosition)
			print("OvertakenVehicleIndex",AmaxScoringEvent.OvertakenVehicleIndex)
			print("--")
			print("")
		end

	elseif type == UIEnums.TrackScoring.ScoringType.FatallyDamagedVehicle then
		
		if ScoringEvent_Debug == 1 then
			print("-- ",GetScoringEventName(type))
			print("Vehicle Index",AmaxScoringEvent.VehicleIndex)
			print("--")
			print("")
		end

	elseif type == UIEnums.TrackScoring.ScoringType.Collision then

		if ScoringEvent_Debug == 1 and AmaxScoringEvent.VehicleIndex == 0 then
			print("-- ",GetScoringEventName(type))
			print("Vehicle Index",AmaxScoringEvent.VehicleIndex)
			print("Damage",AmaxScoringEvent.Damage)
			print("AgressorVehicleIndex",AmaxScoringEvent.AgressorVehicleIndex)
			print("OtherVehicleIndex",AmaxScoringEvent.OtherVehicleIndex)
			print("--")
			print("")
		end

	elseif type == UIEnums.TrackScoring.ScoringType.None then
		--		
	end

end

function UpdateScoringLogic(time)


end

















-- Helper Functions

function GetScoringEventName(type)
	if type == UIEnums.TrackScoring.ScoringType.None then
		return "None"
	elseif type == UIEnums.TrackScoring.ScoringType.UsedPerk then
		return "UsedPerk"
	elseif type == UIEnums.TrackScoring.ScoringType.ShieldedPerk then
		return "ShieldedPerk"
	elseif type == UIEnums.TrackScoring.ScoringType.Overtake then
		return "Overtake"
	elseif type == UIEnums.TrackScoring.ScoringType.FatallyDamagedVehicle then
		return "FatallyDamagedVehicle"
	elseif type == UIEnums.TrackScoring.ScoringType.Collision then
		return "Collision"
	end

	return "Unknown"
end

function GetPerkName(type)
	if type == UIEnums.TrackScoring.PickupType.None then
		return "None"
	elseif type == UIEnums.TrackScoring.PickupType.Nitrous then
		return "Nitrous"
	elseif type == UIEnums.TrackScoring.PickupType.Shock then
		return "Shock"
	elseif type == UIEnums.TrackScoring.PickupType.Shunt then
		return "Shunt"
	elseif type == UIEnums.TrackScoring.PickupType.Shield then
		return "Shield"
	elseif type == UIEnums.TrackScoring.PickupType.Repair then
		return "Repair"
	elseif type == UIEnums.TrackScoring.PickupType.XP then
		return "XP"
	elseif type == UIEnums.TrackScoring.PickupType.Mine then
		return "Mine"
	elseif type == UIEnums.TrackScoring.PickupType.Barge then
		return "Barge"
	end

	return "Unknown"
end

