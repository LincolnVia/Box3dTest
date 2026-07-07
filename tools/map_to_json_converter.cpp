#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
constexpr double Epsilon = 0.001;

struct Vec3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct Bounds
{
    Vec3 min{};
    Vec3 max{};
    bool valid = false;
};

struct Face
{
    Vec3 p1{};
    Vec3 p2{};
    Vec3 p3{};
    Vec3 normal{};
    double distance = 0.0;
    std::string texture;
    std::vector<double> textureValues;
};

struct Brush
{
    int index = 0;
    std::vector<Face> faces;
    std::vector<Vec3> vertices;
    Bounds bounds{};
};

struct Entity
{
    int index = 0;
    std::map<std::string, std::string> properties;
    std::vector<Brush> brushes;
};

struct Options
{
    double scale = 1.0;
    bool quakeZUpToEngineYUp = true;
    std::set<std::string> portalableMaterials;
};

std::string Trim(const std::string& value)
{
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();

    if (begin >= end)
    {
        return {};
    }

    return std::string(begin, end);
}

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool StartsWith(const std::string& value, const std::string& prefix)
{
    return value.rfind(prefix, 0) == 0;
}

std::string StripComment(const std::string& line)
{
    bool inQuote = false;

    for (std::size_t i = 0; i + 1 < line.size(); ++i)
    {
        if (line[i] == '"')
        {
            inQuote = !inQuote;
        }

        if (!inQuote && line[i] == '/' && line[i + 1] == '/')
        {
            return line.substr(0, i);
        }
    }

    return line;
}

std::string EscapeJson(const std::string& value)
{
    std::ostringstream escaped;

    for (const char c : value)
    {
        switch (c)
        {
        case '\\':
            escaped << "\\\\";
            break;
        case '"':
            escaped << "\\\"";
            break;
        case '\n':
            escaped << "\\n";
            break;
        case '\r':
            escaped << "\\r";
            break;
        case '\t':
            escaped << "\\t";
            break;
        default:
            escaped << c;
            break;
        }
    }

    return escaped.str();
}

std::string FormatDouble(double value)
{
    if (std::abs(value) < 0.0000005)
    {
        value = 0.0;
    }

    std::ostringstream out;
    out.setf(std::ios::fixed, std::ios::floatfield);
    out.precision(6);
    out << value;

    std::string text = out.str();
    while (text.size() > 1 && text.back() == '0')
    {
        text.pop_back();
    }

    if (!text.empty() && text.back() == '.')
    {
        text.pop_back();
    }

    return text;
}

std::string WriteVec3(const Vec3& value)
{
    return "[" + FormatDouble(value.x) + ", " + FormatDouble(value.y) + ", " + FormatDouble(value.z) + "]";
}

Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return Vec3{ a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 operator*(const Vec3& value, double scale)
{
    return Vec3{ value.x * scale, value.y * scale, value.z * scale };
}

double Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return Vec3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double Length(const Vec3& value)
{
    return std::sqrt(Dot(value, value));
}

Vec3 Normalize(const Vec3& value)
{
    const double length = Length(value);
    if (length <= Epsilon)
    {
        return {};
    }

    return value * (1.0 / length);
}

Vec3 ConvertCoordinates(const Vec3& value, const Options& options)
{
    if (options.quakeZUpToEngineYUp)
    {
        return Vec3{ value.x * options.scale, value.z * options.scale, value.y * options.scale };
    }

    return value * options.scale;
}

std::vector<double> ParseNumbers(const std::string& value)
{
    static const std::regex numberPattern(R"([-+]?(?:(?:\d+\.\d*)|(?:\.\d+)|(?:\d+))(?:[eE][-+]?\d+)?)");
    std::vector<double> numbers;

    for (auto it = std::sregex_iterator(value.begin(), value.end(), numberPattern); it != std::sregex_iterator(); ++it)
    {
        numbers.push_back(std::stod(it->str()));
    }

    return numbers;
}

std::optional<Vec3> ParseVec3Text(const std::string& value)
{
    const std::vector<double> numbers = ParseNumbers(value);
    if (numbers.size() < 3)
    {
        return std::nullopt;
    }

    return Vec3{ numbers[0], numbers[1], numbers[2] };
}

std::vector<std::string> ExtractParenthesizedGroups(const std::string& line)
{
    std::vector<std::string> groups;

    std::size_t cursor = 0;
    while (groups.size() < 3)
    {
        const std::size_t open = line.find('(', cursor);
        if (open == std::string::npos)
        {
            break;
        }

        const std::size_t close = line.find(')', open + 1);
        if (close == std::string::npos)
        {
            break;
        }

        groups.push_back(line.substr(open + 1, close - open - 1));
        cursor = close + 1;
    }

    return groups;
}

std::string TextAfterThirdParenthesis(const std::string& line)
{
    std::size_t cursor = 0;
    for (int i = 0; i < 3; ++i)
    {
        const std::size_t close = line.find(')', cursor);
        if (close == std::string::npos)
        {
            return {};
        }
        cursor = close + 1;
    }

    return Trim(line.substr(cursor));
}

std::optional<Face> ParseFace(const std::string& line)
{
    const std::vector<std::string> pointGroups = ExtractParenthesizedGroups(line);
    if (pointGroups.size() != 3)
    {
        return std::nullopt;
    }

    const auto p1 = ParseVec3Text(pointGroups[0]);
    const auto p2 = ParseVec3Text(pointGroups[1]);
    const auto p3 = ParseVec3Text(pointGroups[2]);

    if (!p1 || !p2 || !p3)
    {
        return std::nullopt;
    }

    std::istringstream suffix(TextAfterThirdParenthesis(line));
    std::string texture;
    suffix >> texture;

    if (texture.empty())
    {
        texture = "__missing_texture__";
    }

    std::string textureTail;
    std::getline(suffix, textureTail);

    Face face;
    face.p1 = *p1;
    face.p2 = *p2;
    face.p3 = *p3;
    face.texture = texture;
    face.textureValues = ParseNumbers(textureTail);
    face.normal = Normalize(Cross(face.p2 - face.p1, face.p3 - face.p1));
    face.distance = Dot(face.normal, face.p1);

    return face;
}

std::optional<std::pair<std::string, std::string>> ParseProperty(const std::string& line)
{
    static const std::regex propertyPattern(R"map_property(^\s*"([^"]*)"\s+"([^"]*)"\s*$)map_property");
    std::smatch match;

    if (!std::regex_match(line, match, propertyPattern))
    {
        return std::nullopt;
    }

    return std::pair<std::string, std::string>{ match[1].str(), match[2].str() };
}

std::optional<Vec3> IntersectPlanes(const Face& a, const Face& b, const Face& c)
{
    const Vec3 bCrossC = Cross(b.normal, c.normal);
    const double denominator = Dot(a.normal, bCrossC);

    if (std::abs(denominator) <= Epsilon)
    {
        return std::nullopt;
    }

    const Vec3 cCrossA = Cross(c.normal, a.normal);
    const Vec3 aCrossB = Cross(a.normal, b.normal);
    const Vec3 point = (bCrossC * a.distance + cCrossA * b.distance + aCrossB * c.distance) * (1.0 / denominator);

    return point;
}

bool IsInsideBrush(const Vec3& point, const Brush& brush)
{
    for (const Face& face : brush.faces)
    {
        if (Dot(face.normal, point) - face.distance < -Epsilon)
        {
            return false;
        }
    }

    return true;
}

bool HasNearDuplicate(const std::vector<Vec3>& vertices, const Vec3& candidate)
{
    for (const Vec3& vertex : vertices)
    {
        if (Length(vertex - candidate) <= Epsilon)
        {
            return true;
        }
    }

    return false;
}

Bounds ComputeBounds(const std::vector<Vec3>& vertices)
{
    Bounds bounds;
    if (vertices.empty())
    {
        return bounds;
    }

    bounds.min = vertices.front();
    bounds.max = vertices.front();
    bounds.valid = true;

    for (const Vec3& vertex : vertices)
    {
        bounds.min.x = std::min(bounds.min.x, vertex.x);
        bounds.min.y = std::min(bounds.min.y, vertex.y);
        bounds.min.z = std::min(bounds.min.z, vertex.z);

        bounds.max.x = std::max(bounds.max.x, vertex.x);
        bounds.max.y = std::max(bounds.max.y, vertex.y);
        bounds.max.z = std::max(bounds.max.z, vertex.z);
    }

    return bounds;
}

void ComputeBrushGeometry(Brush& brush)
{
    brush.vertices.clear();

    for (std::size_t i = 0; i < brush.faces.size(); ++i)
    {
        for (std::size_t j = i + 1; j < brush.faces.size(); ++j)
        {
            for (std::size_t k = j + 1; k < brush.faces.size(); ++k)
            {
                const auto point = IntersectPlanes(brush.faces[i], brush.faces[j], brush.faces[k]);
                if (point && IsInsideBrush(*point, brush) && !HasNearDuplicate(brush.vertices, *point))
                {
                    brush.vertices.push_back(*point);
                }
            }
        }
    }

    brush.bounds = ComputeBounds(brush.vertices);
}

std::string ReadTextFile(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        throw std::runtime_error("Could not open input file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::vector<Entity> ParseMapFile(const std::filesystem::path& inputPath)
{
    std::istringstream lines(ReadTextFile(inputPath));
    std::vector<Entity> entities;

    Entity currentEntity;
    Brush currentBrush;
    int depth = 0;
    int nextEntityIndex = 0;
    int nextBrushIndex = 0;

    std::string line;
    while (std::getline(lines, line))
    {
        line = Trim(StripComment(line));
        if (line.empty())
        {
            continue;
        }

        if (line == "{")
        {
            if (depth == 0)
            {
                currentEntity = {};
                currentEntity.index = nextEntityIndex++;
                nextBrushIndex = 0;
            }
            else if (depth == 1)
            {
                currentBrush = {};
                currentBrush.index = nextBrushIndex++;
            }

            ++depth;
            continue;
        }

        if (line == "}")
        {
            if (depth == 2)
            {
                ComputeBrushGeometry(currentBrush);
                currentEntity.brushes.push_back(currentBrush);
                currentBrush = {};
            }
            else if (depth == 1)
            {
                entities.push_back(currentEntity);
                currentEntity = {};
            }

            --depth;
            if (depth < 0)
            {
                throw std::runtime_error("Unbalanced closing brace in map file");
            }
            continue;
        }

        if (depth == 1)
        {
            if (const auto property = ParseProperty(line))
            {
                currentEntity.properties[property->first] = property->second;
            }
        }
        else if (depth == 2)
        {
            if (const auto face = ParseFace(line))
            {
                currentBrush.faces.push_back(*face);
            }
        }
    }

    if (depth != 0)
    {
        throw std::runtime_error("Unbalanced opening brace in map file");
    }

    return entities;
}

std::string GetClassname(const Entity& entity)
{
    const auto it = entity.properties.find("classname");
    return it == entity.properties.end() ? "unknown" : it->second;
}

std::string GetDominantMaterial(const Brush& brush)
{
    std::unordered_map<std::string, int> counts;
    std::string firstMaterial;

    for (const Face& face : brush.faces)
    {
        if (firstMaterial.empty())
        {
            firstMaterial = face.texture;
        }

        ++counts[face.texture];
    }

    std::string best = firstMaterial;
    int bestCount = -1;
    for (const auto& [material, count] : counts)
    {
        if (count > bestCount)
        {
            best = material;
            bestCount = count;
        }
    }

    return best;
}

bool IsPortalableMaterial(const std::string& material, const Options& options)
{
    const std::string lower = ToLower(material);
    return options.portalableMaterials.contains(lower) || lower.find("portalable") != std::string::npos;
}

bool IsBrushPortalable(const Brush& brush, const Options& options)
{
    for (const Face& face : brush.faces)
    {
        if (IsPortalableMaterial(face.texture, options))
        {
            return true;
        }
    }

    return false;
}

Vec3 BoundsCenter(const Bounds& bounds)
{
    return Vec3{
        (bounds.min.x + bounds.max.x) * 0.5,
        (bounds.min.y + bounds.max.y) * 0.5,
        (bounds.min.z + bounds.max.z) * 0.5
    };
}

Vec3 BoundsHalfExtents(const Bounds& bounds)
{
    return Vec3{
        (bounds.max.x - bounds.min.x) * 0.5,
        (bounds.max.y - bounds.min.y) * 0.5,
        (bounds.max.z - bounds.min.z) * 0.5
    };
}

void WriteProperties(std::ostream& out, const std::map<std::string, std::string>& properties, const std::string& indent)
{
    out << indent << "\"Properties\": {";
    if (!properties.empty())
    {
        out << "\n";
        std::size_t index = 0;
        for (const auto& [key, value] : properties)
        {
            out << indent << "  \"" << EscapeJson(key) << "\": \"" << EscapeJson(value) << "\"";
            if (++index < properties.size())
            {
                out << ",";
            }
            out << "\n";
        }
        out << indent;
    }
    out << "}";
}

void WriteFaces(std::ostream& out, const Brush& brush, const Options& options, const std::string& indent)
{
    out << indent << "\"Faces\": [";
    if (!brush.faces.empty())
    {
        out << "\n";
        for (std::size_t i = 0; i < brush.faces.size(); ++i)
        {
            const Face& face = brush.faces[i];
            out << indent << "  {\n";
            out << indent << "    \"Texture\": \"" << EscapeJson(face.texture) << "\",\n";
            out << indent << "    \"IsPortalable\": " << (IsPortalableMaterial(face.texture, options) ? "true" : "false") << ",\n";
            out << indent << "    \"Points\": [" << WriteVec3(face.p1) << ", " << WriteVec3(face.p2) << ", " << WriteVec3(face.p3) << "],\n";
            out << indent << "    \"Normal\": " << WriteVec3(face.normal) << ",\n";
            out << indent << "    \"TextureValues\": [";
            for (std::size_t valueIndex = 0; valueIndex < face.textureValues.size(); ++valueIndex)
            {
                if (valueIndex > 0)
                {
                    out << ", ";
                }
                out << FormatDouble(face.textureValues[valueIndex]);
            }
            out << "]\n";
            out << indent << "  }";
            if (i + 1 < brush.faces.size())
            {
                out << ",";
            }
            out << "\n";
        }
        out << indent;
    }
    out << "]";
}

void WriteJson(
    const std::filesystem::path& inputPath,
    const std::filesystem::path& outputPath,
    const std::vector<Entity>& entities,
    const Options& options)
{
    if (outputPath.has_parent_path())
    {
        std::filesystem::create_directories(outputPath.parent_path());
    }

    std::ofstream out(outputPath, std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Could not open output file: " + outputPath.string());
    }

    out << "{\n";
    out << "  \"Format\": \"Box3dTest.TrenchBroomMap\",\n";
    out << "  \"Version\": 1,\n";
    out << "  \"Source\": \"" << EscapeJson(inputPath.string()) << "\",\n";
    out << "  \"CoordinateSystem\": \"" << (options.quakeZUpToEngineYUp ? "QuakeZUpToEngineYUp" : "RawMapCoordinates") << "\",\n";
    out << "  \"Scale\": " << FormatDouble(options.scale) << ",\n";
    out << "  \"EntityConfigPath\": null,\n";
    out << "  \"ModelPaths\": [],\n";
    out << "  \"WorldObjects\": [";

    bool wroteWorldObject = false;
    for (const Entity& entity : entities)
    {
        const std::string classname = GetClassname(entity);

        for (const Brush& brush : entity.brushes)
        {
            if (!brush.bounds.valid)
            {
                continue;
            }

            const Vec3 mapCenter = BoundsCenter(brush.bounds);
            const Vec3 mapHalfExtents = BoundsHalfExtents(brush.bounds);
            const Vec3 center = ConvertCoordinates(mapCenter, options);
            const Vec3 halfExtents = ConvertCoordinates(mapHalfExtents, options);
            const std::string material = GetDominantMaterial(brush);

            out << (wroteWorldObject ? ",\n" : "\n");
            out << "    {\n";
            out << "      \"Name\": \"" << EscapeJson(classname + "_brush_" + std::to_string(brush.index)) << "\",\n";
            out << "      \"SourceEntity\": " << entity.index << ",\n";
            out << "      \"SourceBrush\": " << brush.index << ",\n";
            out << "      \"Classname\": \"" << EscapeJson(classname) << "\",\n";
            out << "      \"Material\": \"" << EscapeJson(material) << "\",\n";
            out << "      \"IsPortalable\": " << (IsBrushPortalable(brush, options) ? "true" : "false") << ",\n";
            out << "      \"VertexCount\": " << brush.vertices.size() << ",\n";
            out << "      \"MapBounds\": {\n";
            out << "        \"Min\": " << WriteVec3(brush.bounds.min) << ",\n";
            out << "        \"Max\": " << WriteVec3(brush.bounds.max) << "\n";
            out << "      },\n";
            out << "      \"Collider\": {\n";
            out << "        \"Type\": \"box\",\n";
            out << "        \"Center\": " << WriteVec3(center) << ",\n";
            out << "        \"HalfExtents\": " << WriteVec3(halfExtents) << "\n";
            out << "      },\n";
            WriteFaces(out, brush, options, "      ");
            out << "\n";
            out << "    }";
            wroteWorldObject = true;
        }
    }

    if (wroteWorldObject)
    {
        out << "\n  ],\n";
    }
    else
    {
        out << "],\n";
    }

    out << "  \"Entities\": [";
    bool wroteEntity = false;

    for (const Entity& entity : entities)
    {
        if (GetClassname(entity) == "worldspawn")
        {
            continue;
        }

        out << (wroteEntity ? ",\n" : "\n");
        out << "    {\n";
        out << "      \"Name\": \"entity_" << entity.index << "\",\n";
        out << "      \"SourceEntity\": " << entity.index << ",\n";
        out << "      \"Classname\": \"" << EscapeJson(GetClassname(entity)) << "\",\n";

        const auto originIt = entity.properties.find("origin");
        if (originIt != entity.properties.end())
        {
            const auto mapOrigin = ParseVec3Text(originIt->second);
            if (mapOrigin)
            {
                out << "      \"MapOrigin\": " << WriteVec3(*mapOrigin) << ",\n";
                out << "      \"Position\": " << WriteVec3(ConvertCoordinates(*mapOrigin, options)) << ",\n";
            }
        }

        WriteProperties(out, entity.properties, "      ");
        out << "\n";
        out << "    }";
        wroteEntity = true;
    }

    if (wroteEntity)
    {
        out << "\n  ]\n";
    }
    else
    {
        out << "]\n";
    }

    out << "}\n";
}

void AddPortalableMaterials(const std::string& csv, Options& options)
{
    std::istringstream values(csv);
    std::string value;

    while (std::getline(values, value, ','))
    {
        value = ToLower(Trim(value));
        if (!value.empty())
        {
            options.portalableMaterials.insert(value);
        }
    }
}

void PrintUsage()
{
    std::cerr
        << "Usage: MapToJsonConverter <input.map> [output.json] [--scale value] [--raw-coordinates] [--portalable-materials tex1,tex2]\n"
        << "\n"
        << "Default coordinates convert TrenchBroom/Quake Z-up [x,y,z] to this engine's Y-up [x,z,y].\n";
}

struct ParsedArguments
{
    std::filesystem::path inputPath;
    std::filesystem::path outputPath;
    Options options;
};

ParsedArguments ParseArguments(int argc, char** argv)
{
    if (argc < 2)
    {
        PrintUsage();
        throw std::runtime_error("Missing input path");
    }

    ParsedArguments arguments;
    arguments.inputPath = argv[1];
    arguments.outputPath = arguments.inputPath;
    arguments.outputPath.replace_extension(".json");

    int index = 2;
    if (index < argc && !StartsWith(argv[index], "--"))
    {
        arguments.outputPath = argv[index];
        ++index;
    }

    while (index < argc)
    {
        const std::string option = argv[index++];

        if (option == "--scale")
        {
            if (index >= argc)
            {
                throw std::runtime_error("--scale requires a numeric value");
            }

            arguments.options.scale = std::stod(argv[index++]);
        }
        else if (option == "--raw-coordinates")
        {
            arguments.options.quakeZUpToEngineYUp = false;
        }
        else if (option == "--portalable-materials")
        {
            if (index >= argc)
            {
                throw std::runtime_error("--portalable-materials requires a comma-separated texture list");
            }

            AddPortalableMaterials(argv[index++], arguments.options);
        }
        else
        {
            throw std::runtime_error("Unknown option: " + option);
        }
    }

    return arguments;
}
}

int main(int argc, char** argv)
{
    try
    {
        const ParsedArguments arguments = ParseArguments(argc, argv);
        const std::vector<Entity> entities = ParseMapFile(arguments.inputPath);

        WriteJson(arguments.inputPath, arguments.outputPath, entities, arguments.options);

        std::cout << "Converted " << arguments.inputPath.string() << " -> " << arguments.outputPath.string() << "\n";
        std::cout << "Entities: " << entities.size() << "\n";
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Conversion failed: " << exception.what() << "\n";
        return 1;
    }

    return 0;
}
