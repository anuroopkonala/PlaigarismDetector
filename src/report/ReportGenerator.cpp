#include "ReportGenerator.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>



std::string ReportGenerator::isoTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto tt  = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string ReportGenerator::riskLabel(double score)
{
    if (score >= 0.8) return "HIGH";
    if (score >= 0.5) return "MEDIUM";
    return "LOW";
}

std::string ReportGenerator::riskColor(double score)
{
    if (score >= 0.8) return "#e74c3c";
    if (score >= 0.5) return "#e67e22";
    return "#27ae60";
}

static std::string escapeJSON(const std::string& s)
{
    std::string out;
    for (char c : s)
    {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else                out += c;
    }
    return out;
}

static std::string escapeHTML(const std::string& s)
{
    std::string out;
    for (char c : s)
    {
        if      (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '&') out += "&amp;";
        else if (c == '"') out += "&quot;";
        else               out += c;
    }
    return out;
}

static std::string basename(const std::string& path)
{
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}



ReportGenerator::ReportGenerator(double threshold)
    : threshold_(threshold)
{}



void ReportGenerator::writeJSON(const std::vector<PairResult>& results,
                                const fs::path&                outFile) const
{
    std::ofstream f(outFile);
    if (!f) throw std::runtime_error("Cannot write: " + outFile.string());

    f << "{\n"
      << "  \"generated\": \"" << isoTimestamp() << "\",\n"
      << "  \"threshold\": "   << threshold_ << ",\n"
      << "  \"totalPairs\": "  << results.size() << ",\n"
      << "  \"pairs\": [\n";

    for (size_t i = 0; i < results.size(); ++i)
    {
        const auto& r = results[i];
        f << "    {\n"
          << "      \"fileA\": \""            << escapeJSON(r.fileA) << "\",\n"
          << "      \"fileB\": \""            << escapeJSON(r.fileB) << "\",\n"
          << "      \"sharedFingerprints\": " << r.sharedFingerprints << ",\n"
          << std::fixed << std::setprecision(4)
          << "      \"lcsSim\": "      << r.lcsSim     << ",\n"
          << "      \"cosineSim\": "   << r.cosineSim  << ",\n"
          << "      \"astSim\": "      << r.astSim     << ",\n"
          << "      \"combinedScore\": " << r.combinedScore << ",\n"
          << "      \"risk\": \""      << riskLabel(r.combinedScore) << "\"\n"
          << "    }" << (i + 1 < results.size() ? "," : "") << "\n";
    }

    f << "  ]\n}\n";
}



static void writeCss(std::ofstream& f)
{
    f << "<style>\n"
      << "*{box-sizing:border-box;margin:0;padding:0}\n"
      << "body{font-family:'Segoe UI',system-ui,sans-serif;background:#f0f2f5;color:#1a1a2e}\n"
      << "header{background:linear-gradient(135deg,#16213e,#0f3460);color:#fff;padding:2rem 2.5rem}\n"
      << "header h1{font-size:1.8rem;font-weight:700}\n"
      << "header p{opacity:.75;margin-top:.4rem;font-size:.9rem}\n"
      << ".stats{display:flex;gap:1rem;padding:1.5rem 2.5rem;flex-wrap:wrap}\n"
      << ".stat{background:#fff;border-radius:10px;padding:1rem 1.5rem;flex:1;min-width:160px;"
         "box-shadow:0 1px 4px rgba(0,0,0,.08)}\n"
      << ".stat .val{font-size:2rem;font-weight:700;color:#0f3460}\n"
      << ".stat .lbl{font-size:.8rem;color:#666;text-transform:uppercase;letter-spacing:.5px}\n"
      << ".container{padding:0 2.5rem 2.5rem}\n"
      << "table{width:100%;border-collapse:collapse;background:#fff;border-radius:10px;"
         "overflow:hidden;box-shadow:0 1px 4px rgba(0,0,0,.08)}\n"
      << "thead{background:#0f3460;color:#fff}\n"
      << "th{padding:.8rem 1rem;text-align:left;font-size:.8rem;text-transform:uppercase;"
         "letter-spacing:.5px;cursor:pointer;user-select:none}\n"
      << "th:hover{background:#16213e}\n"
      << "td{padding:.75rem 1rem;border-bottom:1px solid #f0f2f5;font-size:.875rem}\n"
      << "tr:last-child td{border:none}\n"
      << "tr:hover td{background:#f8f9ff}\n"
      << ".badge{display:inline-block;padding:.2rem .6rem;border-radius:20px;"
         "font-size:.75rem;font-weight:600;color:#fff}\n"
      << ".bar-wrap{background:#e9ecef;border-radius:4px;height:6px;width:100px;"
         "display:inline-block;vertical-align:middle}\n"
      << ".bar{height:6px;border-radius:4px}\n"
      << ".filename{font-family:monospace;font-size:.82rem}\n"
      << ".filter-row{display:flex;gap:1rem;align-items:center;padding:1rem 2.5rem;"
         "background:#fff;border-bottom:1px solid #e9ecef;flex-wrap:wrap}\n"
      << ".filter-row label{font-size:.85rem;color:#555}\n"
      << "input[type=range]{width:160px}\n"
      << "#threshold-val{font-weight:600;color:#0f3460}\n"
      << "#no-results{text-align:center;padding:3rem;color:#888;display:none}\n"
      << "</style>\n";
}

static void writeScript(std::ofstream& f)
{
    f << "<script>\n"
      << "var sortDir={};\n"
      << "function sortTable(col){\n"
      << "  var tbody=document.getElementById('table-body');\n"
      << "  var rows=Array.from(tbody.rows);\n"
      << "  sortDir[col]=!sortDir[col];\n"
      << "  rows.sort(function(a,b){\n"
      << "    var va=a.cells[col].textContent.trim();\n"
      << "    var vb=b.cells[col].textContent.trim();\n"
      << "    var na=parseFloat(va),nb=parseFloat(vb);\n"
      << "    var cmp=isNaN(na)?va.localeCompare(vb):na-nb;\n"
      << "    return sortDir[col]?cmp:-cmp;\n"
      << "  });\n"
      << "  rows.forEach(function(r){tbody.appendChild(r);});\n"
      << "  filterTable();\n"
      << "}\n"
      << "function filterTable(){\n"
      << "  var min=parseFloat(document.getElementById('threshold-slider').value)/100;\n"
      << "  var risk=document.getElementById('risk-filter').value;\n"
      << "  var tbody=document.getElementById('table-body');\n"
      << "  var visible=0;\n"
      << "  for(var i=0;i<tbody.rows.length;i++){\n"
      << "    var row=tbody.rows[i];\n"
      << "    var score=parseFloat(row.getAttribute('data-score'));\n"
      << "    var rv=row.getAttribute('data-risk');\n"
      << "    var show=score>=min&&(!risk||rv===risk);\n"
      << "    row.style.display=show?'':'none';\n"
      << "    if(show)visible++;\n"
      << "  }\n"
      << "  document.getElementById('no-results').style.display=visible?'none':'block';\n"
      << "}\n"
      << "sortTable(5);\n"
      << "</script>\n";
}

static std::string barHtml(double v, const std::string& col)
{
    int p = (int)(v * 100);
    std::ostringstream ss;
    ss << "<div class='bar-wrap'><div class='bar' style='width:" << p
       << "%;background:" << col << "'></div></div> " << p << "%";
    return ss.str();
}

void ReportGenerator::writeHTML(const std::vector<PairResult>& results,
                                const fs::path&                outFile) const
{
    std::ofstream f(outFile);
    if (!f) throw std::runtime_error("Cannot write: " + outFile.string());

    f << "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n"
      << "<meta charset=\"UTF-8\">\n"
      << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
      << "<title>Plagiarism Detection Report</title>\n";

    writeCss(f);

    f << "</head>\n<body>\n"
      << "<header>\n"
      << "<h1>Plagiarism Detection Report</h1>\n"
      << "<p>Generated: " << isoTimestamp()
      << " | Threshold: " << threshold_ << "</p>\n"
      << "</header>\n";

    
    size_t high = 0, med = 0, low = 0;
    for (const auto& r : results)
    {
        if (r.combinedScore >= 0.8) ++high;
        else if (r.combinedScore >= 0.5) ++med;
        else ++low;
    }

    f << "<div class='stats'>\n"
      << "<div class='stat'><div class='val'>"
         << results.size() << "</div><div class='lbl'>Total pairs</div></div>\n"
      << "<div class='stat'><div class='val' style='color:#e74c3c'>"
         << high << "</div><div class='lbl'>High risk</div></div>\n"
      << "<div class='stat'><div class='val' style='color:#e67e22'>"
         << med << "</div><div class='lbl'>Medium risk</div></div>\n"
      << "<div class='stat'><div class='val' style='color:#27ae60'>"
         << low << "</div><div class='lbl'>Low risk</div></div>\n"
      << "</div>\n";

    
    f << "<div class='filter-row'>\n"
      << "<label>Min score: <span id='threshold-val'>0.00</span></label>\n"
      << "<input type='range' id='threshold-slider' min='0' max='100' value='0' "
         "oninput=\"document.getElementById('threshold-val').textContent="
         "(this.value/100).toFixed(2);filterTable()\">\n"
      << "<label>Risk: <select id='risk-filter' onchange='filterTable()'>\n"
      << "<option value=''>All</option>\n"
      << "<option value='HIGH'>High</option>\n"
      << "<option value='MEDIUM'>Medium</option>\n"
      << "<option value='LOW'>Low</option>\n"
      << "</select></label>\n"
      << "</div>\n";

    
    // Table
    f << "<div class='container'>\n"
      << "<table id='results-table'>\n"
      << "<thead><tr>\n"
      << "<th onclick='sortTable(0)'>File A</th>\n"
      << "<th onclick='sortTable(1)'>File B</th>\n"
      << "<th onclick='sortTable(2)'>LCS</th>\n"
      << "<th onclick='sortTable(3)'>Cosine</th>\n"
      << "<th onclick='sortTable(4)'>AST Sim</th>\n"
      << "<th onclick='sortTable(5)'>Combined</th>\n"
      << "<th onclick='sortTable(6)'>Risk</th>\n"
      << "</tr></thead>\n"
      << "<tbody id='table-body'>\n";

    for (const auto& r : results)
    {
        const auto color = riskColor(r.combinedScore);
        const auto label = riskLabel(r.combinedScore);

        f << "<tr data-score='" << std::fixed << std::setprecision(4)
          << r.combinedScore << "' data-risk='" << label << "'>\n"
          << "<td class='filename'>" << escapeHTML(basename(r.fileA)) << "</td>\n"
          << "<td class='filename'>" << escapeHTML(basename(r.fileB)) << "</td>\n"
          << "<td>" << barHtml(r.lcsSim,        "#1abc9c") << "</td>\n"
          << "<td>" << barHtml(r.cosineSim,    "#9b59b6") << "</td>\n"
          << "<td>" << barHtml(r.astSim,       "#e67e22") << "</td>\n"
          << "<td>" << barHtml(r.combinedScore, color)    << "</td>\n"
          << "<td><span class='badge' style='background:" << color
          << "'>" << label << "</span></td>\n"
          << "</tr>\n";
    }

    f << "</tbody>\n</table>\n"
      << "<div id='no-results'>No pairs match the current filter.</div>\n"
      << "</div>\n";

    writeScript(f);

    f << "</body>\n</html>\n";
}



void ReportGenerator::generate(const std::vector<PairResult>& results,
                                const fs::path&                outputDir) const
{
    fs::create_directories(outputDir);
    writeJSON(results, outputDir / "report.json");
    writeHTML(results, outputDir / "report.html");
}
