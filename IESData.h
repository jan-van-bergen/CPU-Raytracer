//#pragma once
//#include <string>
//#include <vector>
//#include <unordered_map>
//#include <sstream>
//#include <fstream>
//
//struct IESParser {
//    std::string IESversion;
//    std::string Manufac;
//    std::string Lumcat;
//    std::string Luminaire;
//    std::string Lamp;
//    std::vector<std::string> Other;
//    std::string Tilt;
//    int NumOfLamps;
//    int LumensPerLamp;
//    int CDMultiplier;
//    int NumVertAngles;
//    int NumHorizAngles;
//    int BallastFactor;
//    double inputWatts;
//    std::vector<double> VertAnglesList;
//    std::vector<double> HorizAnglesList;
//    std::unordered_map<std::unordered_map<double, double>, double> m;
//    int horizTotalCount = 0;
//    int linecount = 0;
//    std::string combinedString = "";
//
//    ///<summary>
//    /// accepts a file input (in form of string) and outputs a matrix of values in the form of
//    /// Dictionary(Dictionary(double,double),double)
//    /// The key of the dictionary is the grid coordinates for the (y,x) values to search for later for interpolation.
//    /// The Valuestore is the actual candela value given by the IES file at the given (y,x) coordinate.
//    /// 
//    /// Based on test .ies files using the IESNA:LM-63-2002 format. *Have tested with many different files
//    /// from many different manufactuers, but due to the exponential number of possibilities, this parser
//    /// might not work 100% every time. Just use caution.
//    ///</summary>
//    ///<param name="iesFile">FilePath for .ies file to parse</param>
//    std::unordered_map<std::unordered_map<double, double>, double> ParseIES(std::string iesFile) {
//		std::ifstream fs("thefile.txt");
//        int count = 0;
//        int res = 0;
//        std::string readLine = "";
//        while (std::getline(fs, readLine)) {
//            readLine = sr.ReadLine();
//            if (readLine.find("IESNA:") != std::string::npos) {
//                IESversion = readLine;
//            } else if (readLine.find("[MANUFAC]") != std::string::npos) {
//                Manufac = readLine;
//            } else if (readLine.find("[LUMCAT]") != std::string::npos) {
//                Lumcat = readLine;
//            } else if (readLine.find("[LUMINAIRE]") != std::string::npos) {
//                Luminaire = readLine;
//			} else if (readLine.find("[LAMP]") != std::string::npos) {
//                Lamp = readLine;
//            }
//
//            if (res == 0 && int.TryParse(readLine.Substring(0, 1), out res))
//            {
//                string[] firstLine = readLine.Trim().Split(' ');
//                int.TryParse(firstLine.First(), out NumOfLamps);
//                int.TryParse(firstLine[1], out LumensPerLamp);
//                int.TryParse(firstLine[3], out NumVertAngles);
//                int.TryParse(firstLine[4], out NumHorizAngles);
//            }
//            else if (res > 0)
//            {
//                string[] line = readLine.TrimEnd().Split(' ');
//                if (line.Count() < 4 && readLine.StartsWith(" ") == false)
//                    double.TryParse(line.Last(), out inputWatts);
//
//                if (readLine.StartsWith("0") || readLine.StartsWith(" ") && HorizAnglesList.Count < NumHorizAngles)
//                {
//                    if (line.Count() > NumHorizAngles && VertAnglesList.Count < NumVertAngles)
//                    {
//                        foreach (string s in line)
//                        {
//                            double dblRes;
//                            double.TryParse(s, out dblRes);
//                            if (VertAnglesList.Contains(dblRes)) { }
//                            else
//                                VertAnglesList.Add(dblRes);
//                        }
//                    }
//                    if (line.Last() == "90")
//                    {
//                        if (line.Count() > NumHorizAngles)
//                            foreach (string s in line)
//                            {
//                                double dblRes;
//                                double.TryParse(s, out dblRes);
//                                if (VertAnglesList.Contains(dblRes)) { }
//                                else
//                                    VertAnglesList.Add(dblRes);
//                            }
//
//                        else if (line.Count() < NumVertAngles && line.First() == "0")
//                            foreach (string s in line)
//                            {
//                                double dblRes;
//                                double.TryParse(s, out dblRes);
//                                HorizAnglesList.Add(dblRes);
//                            }
//                    }
//                }
//                else
//                {
//                    if (!readLine.StartsWith("0") && NumHorizAngles > 0 && line.Last() != inputWatts.ToString())
//                    {
//                        if (line.Last() == "0")
//                        {
//                            combinedString += readLine.Trim() + "%";
//                        }
//                        else
//                        {
//                            combinedString += readLine.Trim();
//                        }
//                    }
//                }
//            }
//            count++;
//        }
//            
//        string[] values = combinedString.Trim().Split('%');
//        foreach (string v in values)
//        {
//            string[] cdValues = v.Trim().Split(' ');
//            foreach (string value in cdValues)
//            {                   
//                double dblRes;
//                if(double.TryParse(value, out dblRes))
//                {
//                    Dictionary<double, double> dictKey = new Dictionary<double, double>();
//                    dictKey.Add(VertAnglesList.ElementAt(linecount), HorizAnglesList.ElementAt(horizTotalCount));
//                    m.Add(dictKey, dblRes);
//                    linecount++;
//                }
//            }
//            horizTotalCount++;
//            linecount = 0;
//        }
//        sr.Close();
//        fs.Close();
//        return m;
//    }
//
//    /// <summary>
//    /// Results printer for the matrix. *In progress*
//    /// Idea is to take the resulting Dictionary from the parser and output to a human readable table
//    /// for verification, printing, etc.
//    /// </summary>
//    /// <param name="m">Dictionary result from the parser</param>
//    /// <returns>human readable string table to printout to console or MessageBox, etc..</returns>
//	std::string ResultIESMatrix(Dictionary<Dictionary<double, double>, double> m) {
//        string sx = "";
//        string sy = "";
//        foreach (KeyValuePair<Dictionary<double, double>, double> i in m)
//        {
//            double x = 0;
//            double y = 0;
//            foreach (KeyValuePair<double, double> pair in i.Key)
//            {
//                y = pair.Key;
//                x = pair.Value;
//            }
//            double candelaValue = i.Value;
//        }
//        return "";
//    }
//}