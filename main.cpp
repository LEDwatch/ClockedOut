#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>

#define PI 3.141592654

void transformModule(std::string& pcb, std::string& module,
	int modStart, int modEnd, float centerX, float centerY, float angle, float radius);

void transformPoint(std::string& pcb, std::string& module, size_t pcbPos,
	size_t modPos, float centerX, float centerY, float angle, float radius);

int findNextPoint(std::string& str, int startPos);

void findNextModule(std::string& str, std::string& modName, int searchPos, int& startPos, int& endPos);

std::string getModuleName(std::string& module);

int main(int argc, char* argv[]) {
	if(argc < 4) {
		std::cout << "Error: need module and PCB file names!" << std::endl;
		return -1;
	}

	float centerX, centerY, radius, totalAngle;
	int partCount;

	std::cout << "CenterX: ";
	std::cin >> centerX;

	std::cout << "\r\nCenterY: ";
	std::cin >> centerY;

	std::cout << "\r\nRadius: ";
	std::cin >> radius;

	std::cout << "\r\nPart Count: ";
	std::cin >> partCount;

	std::cout << "\r\nTotal angle (degrees): ";
	std::cin >> totalAngle;

	std::cout << std::endl;

	//Open the module file
	std::string modFileName = argv[1];
	std::string pcbFileName = argv[2];
	std::ifstream modFile(modFileName);
	std::ifstream pcbFile(pcbFileName);

	std::string module, pcb;

	//Allocate memory for the module file contents
	modFile.seekg(0, std::ios::end);
	module.reserve(modFile.tellg());
	modFile.seekg(0, std::ios::beg);

	//Allocate memory for the pcb file contents
	pcbFile.seekg(0, std::ios::end);
	pcb.reserve(pcbFile.tellg());
	pcbFile.seekg(0, std::ios::beg);

	//Load the file into memory
	module.assign((std::istreambuf_iterator<char>(modFile)),
		std::istreambuf_iterator<char>());

	//Load the pcb file into memory
	pcb.assign((std::istreambuf_iterator<char>(pcbFile)),
		std::istreambuf_iterator<char>());

	modFile.close();
	pcbFile.close();

	std::string moduleName = getModuleName(module);

	std::cout << "Found module name: " << moduleName << std::endl;

	int modStart = 0, modEnd = 0;

	for(int i = 0; i < partCount; i++) {
		double angle = (double) PI * totalAngle * i / 180. / partCount;
		
		double x = centerX + radius*std::cos(angle);
		double y = centerY + radius*std::sin(angle);

		findNextModule(pcb, moduleName, modEnd, modStart, modEnd);

		std::cout << "Found module " << i+1 << " between (" << modStart <<
			", " << modEnd << ")" << std::endl;

		transformModule(pcb, module, modStart, modEnd, x, y, angle, radius);

	}

	std::ofstream outPcb(argv[3]);
	outPcb << pcb;
	outPcb.close();

	return 0;
}
void transformModule(std::string& pcb, std::string& module,
	int modStart, int modEnd, float centerX, float centerY, float angle, float radius) {
	
	int modPos = 0, pcbPos = modStart;

	///while(1) {
		modPos = findNextPoint(module, modPos);
		pcbPos = findNextPoint(pcb, pcbPos);

		//if(modPos >= module.length() || pcbPos >= modEnd)
			//break;

		transformPoint(pcb, module, pcbPos, modPos, centerX, centerY, angle, radius);

		//modPos++;
		//pcbPos++;
	//}
}


void findNextModule(std::string& str, std::string& modName, int searchPos, int& startPos, int& endPos) {
	startPos = str.find(modName, searchPos);

	if(startPos == std::string::npos) {
		startPos = endPos = str.length();

		return;
	}

	int depth = 1;
	endPos = startPos;

	while(depth > 0 && endPos < str.length()) {
		char ch = str[endPos++];

		if(ch == '(')
			depth++;
		else if(ch == ')')
			depth--;
	}

	if(depth != 0) {
		std::cout << "ERROR: EOF found while depth = " << depth << "!" << std::endl;
	}
}


std::string getModuleName(std::string& module) {
	std::stringstream ss(module);
	std::string name;

	ss >> name; //junk
	ss >> name;

	return name;
}


void transformPoint(std::string& pcb, std::string& module, size_t pcbPos, size_t modPos,
	float centerX, float centerY, float angle, float radius) {
	std::stringstream ssPcb(pcb), ssMod(module);

	double cosx = std::cos(angle), sinx = std::sin(angle);

		std::string junk;
		float x, y, newX, newY;
		int modStartPos, modEndPos;
		int pcbStartPos, pcbEndPos;

		ssMod.seekg(modPos, std::ios::beg);
		ssPcb.seekg(pcbPos, std::ios::beg);

		ssMod >> junk;
		modStartPos = ssMod.tellg();
		ssMod >> x;
		ssMod >> y;
		modEndPos = ssMod.tellg();

		ssPcb >> junk;
		pcbStartPos = ssPcb.tellg();
		ssPcb >> junk;
		ssPcb >> junk;

		if(junk[junk.length() - 1] != ')')
			ssPcb >> junk;
		pcbEndPos = ssPcb.tellg();

		//newX = x*cosx - y*sinx + centerX;
		//newY = y*cosx + x*sinx + centerY;
		newX = centerX + radius*std::cos(angle);
		newY = centerY + radius*std::sin(angle);

		std::string tail = "";
		if(junk[junk.length() - 1] == ')')
			tail = ")";

		pcb = pcb.substr(0, pcbStartPos) + " " +
			std::to_string(newX) + " " +
			std::to_string(newY) + " " + 
			std::to_string(360. - angle * 180. / PI) + tail +
			pcb.substr(pcbEndPos, pcb.length() - 1);
}

int findNextPoint(std::string& str, int startPos) {
	size_t atPos, stPos, endPos;

	atPos = str.find("at ", startPos);
	stPos = str.find("start ", startPos);
	endPos = str.find("end ", startPos);

	if(atPos == std::string::npos)
		atPos = str.length();
	if(stPos == std::string::npos)
		stPos = str.length();
	if(endPos == std::string::npos)
		endPos = str.length();
	
	return std::min(atPos, std::min(stPos, endPos));
}
