#include "Main.h"

class CarbonData
{
	std::string getCurrentDate() {
		auto now = std::chrono::system_clock::now();
		std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
		std::tm localTime = {};
		if (localtime_s(&localTime, &currentTime) != 0) {
			std::cerr << "Failed to get local time" << std::endl;
			return "";
		}

		int year = localTime.tm_year + 1900;
		int month = localTime.tm_mon + 1;
		int day = localTime.tm_mday;
		std::stringstream dateString;
		dateString << year << "-" << month << "-" << day;

		return dateString.str();
	}
public:
	int id;
	double energy_emission,
		transport_emission,
		waste_emission,
		totalemission;
	std::string suggestion;
	std::string DateTime = getCurrentDate();
	std::string vehicletype;
	CarbonData(int i = 0, double e = 0, double t = 0, double w = 0, double tt = 0, std::string veh = "", std::string sug = "")
	{
		id = i;
		energy_emission = e;
		transport_emission = t;
		waste_emission = w;
		totalemission = tt;
		vehicletype = veh;
		suggestion = sug;
	}
};//yaan mo muna

class Program
{
	static int callback(void* data, int argc, char** argv, char** azColName) {
		int i;
		for (i = 0; i < argc; i++) {
			std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
		}
		std::cout << std::endl;
		return 0;
	}

	void Execute(sqlite3* db, const char* sql, char* zErrMsg) {
		int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << zErrMsg << std::endl;
			sqlite3_free(zErrMsg);
		}
	}

	void gettime(CarbonData& data) {
		auto start = std::chrono::system_clock::now();
		auto legacyStart = std::chrono::system_clock::to_time_t(start);
		char tmBuff[30];
		ctime_s(tmBuff, sizeof(tmBuff), &legacyStart);
		data.DateTime = tmBuff;
	}

	double calculateDailyEmissions(double time, double averageSpeed, double emissionsFactor[]) {
		double distance = time * averageSpeed;

		if (averageSpeed < 10.0) {
			return distance / emissionsFactor[0];
		}
		else if (averageSpeed <= 20.0) {
			return distance / emissionsFactor[1];
		}
		else {
			return distance / emissionsFactor[2];
		}
	}

	void CalculateEnergyEmission(CarbonData& data)
	{
		double KWH;
		std::cout << "---------------------------------------------\n";
		std::cout << "Please check your Electricity Bill and find KWH used\nEnter KWH : ";
		std::cin >> KWH;
		KWH = KWH * 0.6032;
		data.energy_emission = KWH;
		std::cout << "Your energy emission: " << data.energy_emission << "kg of CO2\n";
	}

	void CalculateTransportEmission(CarbonData& data) {
		const double gasCarSpeed = 29.9;
		const double dieselCarSpeed = 29.9;
		const double dieselJeepneySpeed = 17.57;
		const double dieselBusSpeed = 14.18;
		//double daysOfTravelPerMonth;

		double gasCarEmissions[] = { 58.1, 49.5, 39.4 };
		double dieselCarEmissions[] = { 3.0, 2.5, 2.3 };
		double dieselJeepneyEmissions[] = { 2.9, 2.5, 2.3 };
		double dieselBusEmissions[] = { 11.3, 12.4, 11.3 };

		double timeOfTravel;
		char vehicleType;
		/*start:
		std::cout << "How many times do you go to work, school, or out in a week?\n";
		std::cin >> daysOfTravelPerMonth;
		if (daysOfTravelPerMonth > 7) {
			std::cout << "please enter a number between 1 and 7\n";
			goto start;*/
			//}
		std::cout << "How long does the travel take in minutes? ";
		std::cin >> timeOfTravel;
	CarType:
		std::cout << "Please enter the type of vehicle do you often use to go to work:\ng. Gas Car\nd. Diesel Car\nj. Diesel Jeep\nb. Diesel Bus\n";
		std::cin >> vehicleType;

		vehicleType = toupper(vehicleType);

		double dailyEmissions = 0;
		timeOfTravel /= 60;
		if (vehicleType == 'G') {
			dailyEmissions = calculateDailyEmissions(timeOfTravel, gasCarSpeed, gasCarEmissions);
			data.vehicletype = "GasCar";
		}
		else if (vehicleType == 'D') {
			dailyEmissions = calculateDailyEmissions(timeOfTravel, dieselCarSpeed, dieselCarEmissions);
			data.vehicletype = "DieselCar";
		}
		else if (vehicleType == 'J') {
			dailyEmissions = calculateDailyEmissions(timeOfTravel, dieselJeepneySpeed, dieselJeepneyEmissions);
			data.vehicletype = "DieselJeep";
		}
		else if (vehicleType == 'B') {
			dailyEmissions = calculateDailyEmissions(timeOfTravel, dieselBusSpeed, dieselBusEmissions);
			data.vehicletype = "DieselBus";
		}
		else {
			std::cout << "Invalid vehicle type. Please enter G, D, J, or B." << std::endl;
			goto CarType;
		}
		std::cout << "Your transport emissions: " << (dailyEmissions * 30) / 1000 << " kg of CO2" << std::endl;
		data.transport_emission = (dailyEmissions * 30) / 1000;
		data.vehicletype = vehicleType;
	}

	void CalculateWasteEmission(CarbonData& data) {
		double fdw, pw, gw, ww, tw, waste,
			docf, mcf, f, gwp, fd,
			paper, garden, wood, textile;

		std::cout << "Input the mass(kg) per month of the waste in terms of: \n" << "Food waste: ";
		std::cin >> fdw;
		std::cout << "Paper: ";
		std::cin >> pw;
		std::cout << "Garden: ";
		std::cin >> gw;
		std::cout << "Wood: ";
		std::cin >> ww;
		std::cout << "Textile: ";
		std::cin >> tw;

		docf = 0.5;
		mcf = 1;
		f = 0.5;
		gwp = 28;
		fd = fdw * 0.15 * 0.77 * docf * mcf * f * gwp;
		paper = pw * 0.4 * 0.98 * docf * mcf * f * gwp;
		garden = gw * 0.17 * 0.77 * docf * mcf * f * gwp;
		wood = ww * 0.43 * 0.98 * docf * mcf * f * gwp;
		textile = tw * 0.24 * 0.77 * docf * mcf * f * gwp;
		waste = fd + paper + garden + wood + textile;

		std::cout << "Your waste emission is: " << waste << " kg of CO2.\n";
		data.waste_emission = waste;
	}

	void CalculateTotalEmission(CarbonData& data) {
		data.totalemission = data.energy_emission + data.transport_emission + data.waste_emission;
		std::cout << "Energy Emisison: " << data.energy_emission << std::endl;
		std::cout << "Transport Emisison: " << data.transport_emission << std::endl;
		std::cout << "Waste Emisison: " << data.waste_emission << std::endl;
		std::cout << "Your total carbon emission this month: " << data.totalemission << "kg of CO2\n";
	}

	void SuggestionFunction(CarbonData& data)
	{
		double philippineaverage = 1200 / 12;
		std::string Suggestion;
		std::string energylinks;
		std::cout << "Suggestion to reduce your monthly carbon emission:\n";
		std::cout << "In Electricity:";

		if (data.energy_emission > philippineaverage * .7)
		{
			Suggestion += "\n---if you own any Air conditioning at your household we suggest to reduce it's usage to reduce the its carbon emission. "

				"This can reduce your energy use by 10% and reduce your emission by 200kg per year. You can rely on curtains, fans, or natural "

				"ventilation to cool your home instead of using AC.---\n\n---Switch your appliances such as AC, refirgerators, and washing machine to inverter appliances "

				". Inverter appliances use less electricity than their normal counter parts and produce less heat. Inverter appliances can help you reduce energy consumption "

				"by 50% and reduce your CO2 emission by 1200kg per year---\n\n""---if you are currently using on any incandescent light bulbs we reccomend on replacing them with LED bulbs as "

				"as LED bulbs are 85% more efficient than an incandescent bulbs. By doing so you can reduce your CO2 emissions by 500kg"

				" per year.---\n\n---Use a power strip, a smart plug, or unplug unsused devices. Phantom energy loss accounts about 10% of your household electricity "

				"use. Phantom loss is the energy loss in devices that is still plugged in even though the devices are powered off. By using smart plugs "

				"or unplugging devices that isn't in use can reduce your household electricity and reduce over 400kg of CO2 per year.---\n\n";//5 suggestion

			energylinks += "https://www.metrobank.com.ph/articles/learn/how-to-save-on-electrical-bills \n  https://www.moneymax.ph/lifestyle/articles/home-energy-saving-tips \n"
				" https://www.doe.gov.ph/energy-efficiency/overview \n";
		}
		if (data.energy_emission < philippineaverage * .7) {
			Suggestion += "\n---Installing solar panels around your power your home can reduce your energy consumption therefore your carbon emission. "

				"We suggest to use CBH Solar Light as it offers 120 degrees of ilumination adn 2600 lumens of brghtness and it can also provide "

				"as it can provide 10 hours of light. Using renewable resources can really help you reduce your carbon emission.---\n\n";//1 suggestion

			energylinks += "https://www.doe.gov.ph/energy-efficiency/overview \n";
		}

		if (data.transport_emission > philippineaverage * .01)
		{
			Suggestion += "";//same here
		}
		if (data.transport_emission < philippineaverage * .01)
		{
			Suggestion += "";//same here
		}
		//dito ka kalel maggawa lagay mo nalang sa mga colon yung suggestion mo tsaka dapat may sources ka
		if (data.waste_emission > philippineaverage * .2)
		{
			Suggestion += "";//same here
		}
		if (data.waste_emission < philippineaverage * .2)
		{
			Suggestion += "";//same here
		}
		std::cout << Suggestion << " \n sources \n " << energylinks;
		data.suggestion = Suggestion;
	}

	int CalculatingEmission(sqlite3* db, CarbonData& data)
	{
		int rc;
		const char* sql;
		sqlite3_stmt* stmt;

		CalculateEnergyEmission(data);
		system("cls");
		CalculateTransportEmission(data);
		system("cls");
		CalculateWasteEmission(data);
		system("cls");
		CalculateTotalEmission(data);
		SuggestionFunction(data);

		sql = "INSERT INTO carbondata (energy_emission, transport_emission, waste_emission, total_emission, vehicle_type, suggestions, date) VALUEs ( ?, ?, ?, ?, ?, ?, ?);";
		rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_bind_double(stmt, 1, data.energy_emission);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_bind_double(stmt, 2, data.transport_emission);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_bind_double(stmt, 3, data.waste_emission);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_bind_double(stmt, 4, data.totalemission);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_bind_text(stmt, 5, data.vehicletype.c_str(), -1, NULL);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_bind_text(stmt, 6, data.suggestion.c_str(), -1, NULL);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_bind_text(stmt, 7, data.DateTime.c_str(), -1, NULL);
		if (rc != SQLITE_OK) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			return(-1);
		}
		else {
			std::cout << "Record inserted successfully" << std::endl;
		}
		sqlite3_finalize(stmt);
		system("pause");
		system("cls");
		return 0;
	}

	std::map<std::string, CarbonData> query_db(sqlite3* db, std::string sql) {
		std::map<std::string, CarbonData> results;
		sqlite3_stmt* stmt;
		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		if (rc != SQLITE_OK) {
			std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
			return results;
		}
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			int id = sqlite3_column_int(stmt, 0);
			double energy_emission = sqlite3_column_double(stmt, 1);
			double transport_emission = sqlite3_column_double(stmt, 2);
			double waste_emission = sqlite3_column_double(stmt, 3);
			double totalemission = sqlite3_column_double(stmt, 4);
			std::string vehicle_type = (char*)sqlite3_column_text(stmt, 5);
			std::string suggestion = (char*)sqlite3_column_text(stmt, 6);
			std::string DateTime = (char*)sqlite3_column_text(stmt, 7);
			CarbonData cd(id, energy_emission, transport_emission, waste_emission, totalemission, vehicle_type, suggestion);
			results.insert({ DateTime, cd });
		}
		if (rc != SQLITE_DONE) {
			std::cerr << "Error executing statement: " << sqlite3_errmsg(db) << std::endl;
		}
		sqlite3_finalize(stmt);
		return results;
	}

	void EnergyReccomendation()
	{
		std::cout << "Reducing energy consumption and emission\n\n";
		std::cout << "---if you own any Air conditioning at your household we suggest to reduce it's usage to reduce the its carbon emission. "

			"This can reduce your energy use by 10% and reduce your emission by 200kg per year. You can rely on curtains, fans, or natural "

			"ventilation to cool your home instead of using AC.---\n\n"

			" https://www.moneymax.ph/lifestyle/articles/home-energy-saving-tips \n\n";
		std::cout << "---Switch your appliances such as AC, refirgerators, and washing machine to inverter appliances "

			". Inverter appliances use less electricity than their normal counter parts and produce less heat. Inverter appliances can help you reduce energy consumption "

			"by 50% and reduce your CO2 emission by 1200kg per year---\n\n"

			"https://www.metrobank.com.ph/articles/learn/how-to-save-on-electrical-bills \n\n";
		std::cout << "---if you are currently using on any incandescent light bulbs we reccomend on replacing them with LED bulbs as "

			"as LED bulbs are 85% more efficient than an incandescent bulbs. By doing so you can reduce your CO2 emissions by 500kg"

			" per year.---\n\n"

			"https://www.metrobank.com.ph/articles/learn/how-to-save-on-electrical-bills \n\n";
		std::cout << "---Use a power strip, a smart plug, or unplug unsused devices. Phantom energy loss accounts about 10% of your household electricity "

			"use. Phantom loss is the energy loss in devices that is still plugged in even though the devices are powered off. By using smart plugs "

			"or unplugging devices that isn't in use can reduce your household electricity and reduce over 400kg of CO2 per year.---\n\n"

			"https://www.metrobank.com.ph/articles/learn/how-to-save-on-electrical-bills \n\n";
		std::cout << "---Installing solar panels around your power your home can reduce your energy consumption therefore your carbon emission. "

			"We suggest to use CBH Solar Light as it offers 120 degrees of ilumination adn 2600 lumens of brghtness and it can also provide "

			"as it can provide 10 hours of light. Using renewable resources can really help you reduce your carbon emission.---\n\n";
	}

	void TransportReccomendation()
	{
	}
	void WasteReccomendation()
	{
	}
	int all_reccomendation()
	{
		int input;
		std::cout << "All recommendation to reduce carbon emission and lower carbon footprint\n";
		std::cout << "1.)Energy\n2.)Transport\n3.)Waste\n";
		std::cout << "Enter a number to access a specified reccomendation: ";
		std::cin >> input;

		switch (input)
		{
		case 1:
			system("cls");
			EnergyReccomendation();
			break;
		case 2:
			system("cls");
			break;
		case 3:
			system("cls");
			break;
		case 4:
			system("cls");
			Main();
			break;
		default:
			break;
		}

		system("pause");
		system("cls");
		return 1;
	}
	void Statistic(sqlite3* db, CarbonData& data)
	{
		const char* sql;
		int input;
		std::string inputstring;
		sql = "SELECT * FROM carbondata;";
		std::map<std::string, CarbonData> carbondata = query_db(db, sql);

		std::cout << "Which do you want to access?\n1.) Certain Record\n2.) Get all record\n";
		std::cin >> input;
		switch (input)
		{
		case 1:
		TryAgain:
			std::cout << "Enter a date (yyy-mm-dd): ";
			std::cin >> inputstring;
			for (const auto& pair : carbondata)
			{
				if (pair.first == inputstring) {
					std::cout << "Date: " << pair.first << std::endl << "Total Emision: " << pair.second.totalemission << "kg" << std::endl;
					std::cout << "Energy Emission: " << pair.second.energy_emission << "kg" << std::endl;
					std::cout << "Vehicle Type: " << pair.second.vehicletype << "kg" << std::endl;
					std::cout << "Transport Emission: " << pair.second.transport_emission << "kg" << std::endl;
					std::cout << "Waste Emission: " << pair.second.waste_emission << "kg" << std::endl;
					continue;
				}
			}
			std::cout << "Find another one? (y/N): ";
			std::cin >> inputstring;
			if (inputstring == "y")
				goto TryAgain;
			else
				break;
			break;
		case 2:
			for (const auto& pair : carbondata)
			{
				std::cout << "Date: " << pair.first << std::endl << "Total Emision: " << pair.second.totalemission << "kg" << std::endl;
				std::cout << "Energy Emission: " << pair.second.energy_emission << "kg" << std::endl;
				std::cout << "Vehicle Type: " << pair.second.vehicletype << "kg" << std::endl;
				std::cout << "Transport Emission: " << pair.second.transport_emission << "kg" << std::endl;
				std::cout << "Waste Emission: " << pair.second.waste_emission << "kg" << std::endl;
			}
		default:
			break;
		}
		system("pause");
		system("cls");
	}
	void menu(sqlite3* db, CarbonData& data)
	{
		unsigned int input;
		std::cout << char(218);      for (int i = 0; i < 33; i++) { std::cout << char(196); }      std::cout << char(191) << std::endl;
		std::cout << char(179) << "                                 " << char(179) << std::endl;
		std::cout << char(179) << "                                 " << char(179) << std::endl;
		std::cout << char(179) << "   Carbon FootPrint Calculator   " << char(179) << std::endl;
		std::cout << char(179) << "                                 " << char(179) << std::endl;
		std::cout << char(179) << "                                 " << char(179) << std::endl;
		std::cout << char(195);      for (int i = 0; i < 33; i++) { std::cout << char(196); }      std::cout << char(180) << std::endl;
		std::cout << char(179) << " 1) Enter a new record           " << char(179) << std::endl;
		std::cout << char(195);      for (int i = 0; i < 33; i++) { std::cout << char(196); }      std::cout << char(180) << std::endl;
		std::cout << char(179) << " 2) View all records             " << char(179) << std::endl;
		std::cout << char(195);      for (int i = 0; i < 33; i++) { std::cout << char(196); }      std::cout << char(180) << std::endl;
		std::cout << char(179) << " 3) View suggestions and sources " << char(179) << std::endl;
		std::cout << char(195);      for (int i = 0; i < 33; i++) { std::cout << char(196); }      std::cout << char(180) << std::endl;
		std::cout << char(179) << " 4) Exit                         " << char(179) << std::endl;
		std::cout << char(192);      for (int i = 0; i < 33; i++) { std::cout << char(196); }      std::cout << char(217) << std::endl << " ";

		std::cin >> input;
		switch (input)
		{
		case 1:
			system("cls");
			CalculatingEmission(db, data);
			break;
		case 2:
			system("cls");
			Statistic(db, data);
			break;
		case 3:
			system("cls");
			all_reccomendation();
			break;
		default:
			return exit(1);
			break;
		}
	}
public:

	int Main()
	{
		CarbonData* data = new CarbonData();
		sqlite3* db;
		char* zErrMsg = 0;
		int rc;
		const char* sql;

		rc = sqlite3_open("calculator.db", &db);

		//do not touch please
		if (rc) {
			std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
			return (0);
		}
		sql = "CREATE TABLE IF NOT EXISTS carbondata(" \
			"id INTEGER PRIMARY KEY AUTOINCREMENT," \
			"energy_emission DOUBLE NOT NULL," \
			"transport_emission DOUBLE NOT NULL," \
			"waste_emission DOUBLE NOT NULL,"\
			"total_emission DOUBLE NOT NULL,"\
			"vehicle_type TEXT NOT NULL,"\
			"suggestions TEXT,"\
			"date TEXT NOT NULL);";

		Execute(db, sql, zErrMsg);

		menu(db, *data);

		sqlite3_close(db);
		return 0;
	}
};

int main()
{
	system("COLOR 2F");
	Program program;
	while (true)
	{
		program.Main();
	}
	return 0;
}