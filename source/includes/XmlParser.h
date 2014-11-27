
class XmlParser
{
	private:

	 
	public:
	    XmlParser();
	    int ParseLevelData(std::string levelXml, std::string prefabs); // Will return an array of structs
	    int ParsePrefabs(std::string prefabs); // Will return an array of structs
	    
};