#ifndef MIME_TYPE_HPP_INCLUDED
#define MIME_TYPE_HPP_INCLUDED

namespace web_server
{

  // Return a reasonable mime type based on the extension of a file.
  std::string_view mime_type(std::filesystem::path const& extension)
  {
    std::string ext{extension.string()};
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if(ext == ".html") return "text/html";
    if(ext == ".css")  return "text/css";
    if(ext == ".js")   return "application/javascript";
    if(ext == ".json") return "application/json";
    if(ext == ".xml")  return "application/xml";
    if(ext == ".png")  return "image/png";
    if(ext == ".jpeg") return "image/jpeg";
    if(ext == ".jpg")  return "image/jpeg";
    return "application/text";
  }

} // ns

#endif // MIME_TYPE_HPP_INCLUDED