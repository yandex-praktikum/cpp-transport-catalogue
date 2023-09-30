#pragma once

#include <iostream>
#include <set>
#include "transport_catalogue.h"
#include "stat_reader.h"



/* структуры с промежуточной информацией перед добавлением в транспортный справочник*/

struct BusBuf
{
	std::string name;                                              /* название маршрута */
	std::vector<std::string> busStops;                             /* названия остановок на маршруте */
	size_t unique_stops;                                           /* количество уникальных остановок на маршруте */
};

struct StopBuf
{
	Stop stop;                                                     /* название остановки */
	std::unordered_map<std::string, unsigned int> stopsDistance;   /* дистанции до соседних остановок */
	
};

/*
*========================================================================================
* Функция: isNumeric
* Описание: Функция определяет является ли строка цифрой
* Параметры:
*   str - ссылка на строку 
*
* Возвращаемое значение:
*   1 - строка является цифрой, 0 - строка содержит символы не являющиеся цифрой
*========================================================================================
*/

bool isNumeric(std::string const& str)
{
	return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

/*
*========================================================================================
* Функция: StringToBusStop
* Описание: Обработка запроса на добавление автобусной остановки
* Параметры:
*   str - ссылка на строку запроса
*
* Возвращаемое значение:
*   Структура StopBuf с названием остановки и с дистанциями до соседних остановок
*========================================================================================
*/

StopBuf StringToStop(std::string str)
{
	StopBuf result;
	std::string name;
	std::string lat;
	std::string lng;
	std::string near_bus;
	unsigned int dist;

	size_t pos1 = str.find_first_not_of("Stop");
	size_t pos2 = str.find_first_of(":");
	size_t pos3 = str.find_first_of(",");
	size_t pos4 = str.find_first_of(",", pos3 + 2);
	size_t pos5;

	name = str.substr(str.find_first_not_of(" ", pos1), pos2 - pos1 - 1);
	lat = str.substr(str.find_first_not_of(" ", pos2 + 1), pos3 - pos2 - 2);
	lng = str.substr(pos3 + 2, pos4 - pos3 -2);
	result.stop.name = name;
	result.stop.coord.lat = std::stod(lat);
	result.stop.coord.lng = std::stod(lng);

	while (std::string::npos != pos4)
	{
		pos5 = str.find_first_not_of(" ", pos4 + 1);
		dist = stoi(str.substr(pos5, str.find_first_of("m", pos5)));
		pos5 = str.find_first_of("to", pos5);
		pos4 = str.find_first_of(",", pos4 + 1);
		near_bus = str.substr(str.find_first_not_of(" ", pos5 + 2), pos4 == std::string::npos ? std::string::npos : pos4 - pos5 - 3);
		result.stopsDistance[near_bus] = dist;
	}

	return result;
}



/*
*========================================================================================
* Функция: StringToBusBuf
* Описание: Обработка запроса на добавление автобусного маршрута
* Параметры:
*   str - ссылка на строку запроса
*
* Возвращаемое значение:
*   Структура BusBuf с названием маршрута и остановками
*========================================================================================
*/

BusBuf StringToBusBuf(std::string& str)
{
	
	std::string name;
	std::vector<std::string> tmp;
	std::set<std::string> uniqueStops;
	size_t pos1 = str.find_first_not_of("Bus");
	size_t pos2 = str.find_first_of(":");
	size_t pos3 = str.find_first_of("-");
	size_t pos4 = str.find_first_of(">");

	name = str.substr(str.find_first_not_of(" ", pos1), pos2 - pos1 - 1);
	// если формат автобусного маршрута с "-"
	if (std::string::npos != pos3) 
	{
		tmp.push_back(str.substr(str.find_first_not_of(" ", pos2 + 1), str.find_first_of("-", pos2 + 1) - pos2 - 3));
		while (std::string::npos != pos3)
		{
			tmp.push_back(str.substr(str.find_first_not_of(" ", pos3 + 1), str.find_first_of("-",pos3 + 1 ) - pos3 - 3));
			pos3 = str.find_first_of("-", pos3 + 1);
		}

		size_t size_tmp = tmp.size();

		for (size_t i = size_tmp - 1; i > 0; i--)
		{
			tmp.push_back(tmp[i-1]);
		}

	}
	// если формат автобусного маршрута с ">"
	else if (std::string::npos != pos4)
	{
		tmp.push_back(str.substr(str.find_first_not_of(" ", pos2 + 1), str.find_first_of(">", pos2 + 1) - pos2 - 3));
		while (std::string::npos != pos4)
		{
			tmp.push_back(str.substr(str.find_first_not_of(" ", pos4 + 1), str.find_first_of(">", pos4 + 1) - pos4 - 3));
			pos4 = str.find_first_of(">", pos4 + 1);
		}
	}
	
	for (std::string& str : tmp)
	{
		uniqueStops.insert(str);
	}

	return {name, tmp, uniqueStops.size()};

}

/*
*========================================================================================
* Функция: Load
* Описание: Обработка запросов и добавление информации в транспортный справочник 
* Параметры:
*   input - входной поток
*   transportCatalogue - транспортный справочник
* Возвращаемое значение:
*   нет
*========================================================================================
*/
void Load(std::istream& input, TransportCatalogue& transportCatalogue)
{
	
	std::string words;
	std::string data;
	std::vector<std::string> inputBuf;
	std::vector<std::string> outputBuf;
	BusBuf busBuf;
	std::vector<StopBuf> stopsBuf;
	Bus bus;
	bool addRequest = false;
	bool outputRequest = false;
	int n;
	
	while (std::getline(input, words))
	{
		data = words.substr(0, words.find_first_of(' '));
		if (isNumeric(data)) 
		{
			n = stoi(data);

			for (int i = 0; i < n; i++)
			{
				getline(input, words);
	
				if (std::string::npos != words.find_first_of(":"))
				{
					outputRequest = false;
					inputBuf.push_back(words);
					addRequest = true;
				}
				else if ((std::string::npos != words.find_first_of("Bus")) ||
					     (std::string::npos != words.find_first_of("Stop")))
					
				{
					addRequest = false;
					outputBuf.push_back(words);
					outputRequest = true;
				}

			}

			if (addRequest)
			{
				for (std::string str : inputBuf)
				{
					data = str.substr(0, str.find_first_of(' '));
					if (data == "Stop")
					{
						stopsBuf.push_back(StringToStop(str));
						transportCatalogue.AddStop(stopsBuf.back().stop);
					}
				}

				for (auto stopBuf : stopsBuf)
				{
					transportCatalogue.AddDistance(stopBuf.stop.name, stopBuf.stopsDistance);
				}
				

				for (std::string str : inputBuf)
				{
					data = str.substr(0, str.find_first_of(' '));
					 if (data == "Bus")
					{
						bus.busStops.clear();
						bus.name.clear();
						busBuf = StringToBusBuf(str);
						bus.name = busBuf.name;
						bus.unique_stops = busBuf.unique_stops;
						for (std::string& str : busBuf.busStops)
						{
							bus.busStops.push_back(transportCatalogue.FindStop(str));
						}
						transportCatalogue.AddBus(bus);
					}
				}

			}

			if (outputRequest)
			{
				for (std::string str : outputBuf)
				{
					transportCatalogue.AddOutputRequest(str);
				}
			}



		}
	}
}
// место для вашего кода
