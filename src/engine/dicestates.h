/*  
    Kubix - 3D OpenGL implementation of the board game "Tactix"/"Duell" with AI
    Copyright (C) 2011  Florian Sittel & Carsten Burgard

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define __dicestates_h


char* statePic[26];

void initStatePics(){

	statePic[0]=	 "   o-----o\n"
			"  /  1  /|\n"
			" o-----o |\n"
			" |     |3|\n"
			" |  2  | o\n"
			" |     |/ \n"
			" o-----o\n";
	
	statePic[1]=	 "   o-----o\n"
			"  /  1  /|\n"
			" o-----o |\n"
			" |     |5|\n"
			" |  3  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[2]=	 "   o-----o\n"
			"  /  1  /|\n"
			" o-----o |\n"
			" |     |4|\n"
			" |  5  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[3]=	 "   o-----o\n"
			"  /  1  /|\n"
			" o-----o |\n"
			" |     |2|\n"
			" |  4  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[4]=	 "   o-----o\n"
			"  /  2  /|\n"
			" o-----o |\n"
			" |     |3|\n"
			" |  6  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[5]=	 "   o-----o\n"
			"  /  2  /|\n"
			" o-----o |\n"
			" |     |1|\n"
			" |  3  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[6]=	 "   o-----o\n"
			"  /  2  /|\n"
			" o-----o |\n"
			" |     |4|\n"
			" |  1  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[7]=	 "   o-----o\n"
			"  /  2  /|\n"
			" o-----o |\n"
			" |     |6|\n"
			" |  4  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[8]=	 "   o-----o\n"
			"  /  3  /|\n"
			" o-----o |\n"
			" |     |6|\n"
			" |  2  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[9]=	 "   o-----o\n"
			"  /  3  /|\n"
			" o-----o |\n"
			" |     |5|\n"
			" |  6  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[10]=	"   o-----o\n"
			"  /  3  /|\n"
			" o-----o |\n"
			" |     |2|\n"
			" |  1  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[11]=	"   o-----o\n"
			"  /  3  /|\n"
			" o-----o |\n"
			" |     |1|\n"
			" |  5  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[12]=	"   o-----o\n"
			"  /  4  /|\n"
			" o-----o |\n"
			" |     |1|\n"
			" |  2  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[13]=	"   o-----o\n"
			"  /  4  /|\n"
			" o-----o |\n"
			" |     |2|\n"
			" |  6  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[14]=	"   o-----o\n"
			"  /  4  /|\n"
			" o-----o |\n"
			" |     |5|\n"
			" |  1  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[15]=	"   o-----o\n"
			"  /  4  /|\n"
			" o-----o |\n"
			" |     |6|\n"
			" |  5  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[16]=	"   o-----o\n"
			"  /  5  /|\n"
			" o-----o |\n"
			" |     |3|\n"
			" |  1  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[17]=	"   o-----o\n"
			"  /  5  /|\n"
			" o-----o |\n"
			" |     |1|\n"
			" |  4  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[18]=	"   o-----o\n"
			"  /  5  /|\n"
			" o-----o |\n"
			" |     |4|\n"
			" |  6  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[19]=	"   o-----o\n"
			"  /  5  /|\n"
			" o-----o |\n"
			" |     |6|\n"
			" |  3  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[20]=	"   o-----o\n"
			"  /  6  /|\n"
			" o-----o |\n"
			" |     |3|\n"
			" |  5  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[21]=	"   o-----o\n"
			"  /  6  /|\n"
			" o-----o |\n"
			" |     |4|\n"
			" |  2  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[22]=	"   o-----o\n"
			"  /  6  /|\n"
			" o-----o |\n"
			" |     |2|\n"
			" |  3  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[23]=	"   o-----o\n"
			"  /  6  /|\n"
			" o-----o |\n"
			" |     |5|\n"
			" |  4  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[24]=	"   o-----o\n"
			"  /  1  /|\n"
			" o-----o |\n"
			" |     |1|\n"
			" |  1  | o\n"
			" |     |/\n"
			" o-----o\n";
	
	statePic[25]=	"   o-----o\n"
			"  /  0  /|\n"
			" o-----o |\n"
			" |     |0|\n"
			" |  0  | o\n"
			" |     |/\n"
			" o-----o\n";
}
