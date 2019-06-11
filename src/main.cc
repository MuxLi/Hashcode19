////////////////////////////////////////////
// Google Hashcode 2019                   //
// Slideshow                              //
//                                        //
// @author Muxingzi Li, Cédric Portaneri  // 
////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <algorithm>

typedef int PhotoID;
typedef int TagID;

class Photo {
 public :
  Photo() : used_(false) {}
  ~Photo() {}

  const std::vector<PhotoID>& input_ids() const { return input_ids_; }  
  PhotoID id() const { return id_; }
  bool is_used() const { return used_; }
  const std::set<TagID>& tags() const { return tags_; }
  
  void set_id(PhotoID id) { id_ = id; }
  void add_input_ids_(PhotoID id) { input_ids_.push_back(id); }
  void set_used(bool used) { used_ = used; }
  void add_tag(TagID tag) { tags_.insert(tag); }
  
 private :
  // We merge two vertical photos into one unique photo
  PhotoID id_;                      // unique id after merge
  std::vector<PhotoID> input_ids_;  // photos ids before any merge
  bool used_;                       // true if photo already in slideshow
  std::set<TagID> tags_;            // photo tag ids
};

class Slide {
 public :
  Slide() {}
  Slide(Photo photo) {
    photo_id_ = photo.id();
    for(int tag : photo.tags()) {
      add_tag(tag);
    }    
  }
  ~Slide() {}

  PhotoID photo_id() const { return photo_id_; }  
  const std::set<TagID>& tags() const { return tags_; }
  
  void set_photo_id(PhotoID photo_id) { photo_id_ = photo_id; }
  void add_tag(TagID tag) { tags_.insert(tag); }
  
 private :
  PhotoID photo_id_;      // slide photo id 
  std::set<TagID> tags_;  // all slide tags id 
};

bool sort_by_second(const std::pair<PhotoID,int> &a, const std::pair<PhotoID,int> &b) { 
  return (a.second < b.second); 
} 

/**
 * Read input photos tags
 * @param input_stream: input file stream containing photo tag data
 * @param photo: current photo
 * @param tag_id: current tag id
 * @param tag_str_to_id: output map to convert tag string to tag id
 * @param tag_to_photo: output map to search for a set of photo ids containing a certain tag
 */
void read_photo_tags(std::istringstream &input_stream,
                     Photo &photo, 
                     TagID &tag_id,
                     std::map<std::string, TagID> &tag_str_to_id,
                     std::map<TagID,std::set<PhotoID>> &tag_to_photo) {
  int num_tags = 0;
  input_stream >> num_tags;
  for(int i = 0; i < num_tags; i++) {
    std::string tag;
    input_stream >> tag;
    
    // add tag to current photo
    auto find_tag = tag_str_to_id.find(tag);
    TagID curr_tag_id = 0; 
    if(find_tag == tag_str_to_id.end()) {
      
      // update tag_id and tag_str_to_id
      tag_str_to_id.emplace(tag, tag_id);
      curr_tag_id = tag_id;
      tag_id++;
    }
    else {
      curr_tag_id = find_tag->second;
    }
    photo.add_tag(curr_tag_id);
    
    // update the tag_to_photo map
    auto find_tag_id = tag_to_photo.find(curr_tag_id);
    if(find_tag_id == tag_to_photo.end()) {
      std::set<PhotoID> new_photo_id_set;
      new_photo_id_set.insert(photo.id());
      tag_to_photo.emplace(curr_tag_id, new_photo_id_set);
    }
    else {
      find_tag_id->second.insert(photo.id());
    }
  }  
}

/**
 * Read input photos and build a map to look for photos containing a certain tag
 * @param input_filename: file name of the input data
 * @param input_photos: output map storing each input photos indexed by their id
 * @param tag_to_photo: output map to search for a set of photo ids containing a certain tag
 */
void read_input(const std::string &input_filename, 
                std::map<PhotoID, Photo>& input_photos, 
                std::map<TagID,std::set<PhotoID>> &tag_to_photo) {
  std::ifstream input_file(input_filename);
  int num_photo = 0;
  input_file >> num_photo;
  input_file.ignore();

  std::map<std::string, TagID> tag_str_to_id;
  PhotoID id_after_merge = 0;
  PhotoID input_id = 0;
  TagID tag_id = 0;
  Photo tmp_vertical_photo;
  bool vertical_ready_to_merge = false; // do not add first vertical photo encounter
  
  std::string line;
  while (std::getline(input_file, line)) {
    Photo new_photo;
    bool add_photo = false; 
    
    std::istringstream line_stream(line);
    char orientation;
    line_stream >> orientation;
    if(orientation == 'H') {
      add_photo = true;
      new_photo.set_id(id_after_merge);
      new_photo.add_input_ids_(input_id);
      read_photo_tags(line_stream, new_photo, tag_id, tag_str_to_id, tag_to_photo);
      id_after_merge++;
    }
    else { // Vertical photos
      if(vertical_ready_to_merge) {
        add_photo = true;
        tmp_vertical_photo.add_input_ids_(input_id);
        read_photo_tags(line_stream, tmp_vertical_photo, tag_id, tag_str_to_id, tag_to_photo);
        vertical_ready_to_merge = false;    
        new_photo = tmp_vertical_photo;
        Photo empty;
        tmp_vertical_photo = empty;
      }
      else {
        tmp_vertical_photo.set_id(id_after_merge);
        tmp_vertical_photo.add_input_ids_(input_id);
        read_photo_tags(line_stream, tmp_vertical_photo, tag_id, tag_str_to_id, tag_to_photo);
        id_after_merge++;
        vertical_ready_to_merge = true;
      }
    }
    
    if(add_photo) {
      input_photos.emplace(new_photo.id(), new_photo);
    }
    input_id++;
  }
  input_file.close();
}

/**
 * Write output slideshow into Google specific file format
 * @param input_photos: input photo data
 * @param slideshow: slideshow to write on file
 * @param output_filename: output file name
 */
void write_output(const std::map<PhotoID, Photo>& input_photos, 
                  const std::vector<Slide> &slideshow, 
                  const std::string &output_filename) {
  std::ofstream output_file(output_filename);
  output_file << slideshow.size() << "\n";
  for(const Slide &slide : slideshow) {
    auto find = input_photos.find(slide.photo_id());
    if (find != input_photos.end()) {
      Photo photo = find->second;
      for(PhotoID input_id : photo.input_ids()) {
        output_file << input_id << " ";
      }
      output_file << "\n";
    }
  }
  output_file.close();
}

/**
 * Insert next slide in the slideshow
 * @param current_slide: current slide which we will link with a new slide
 * @param input_photos: map storing each input photos indexed by their id
 * @param tag_to_photo: map to search for a set of photo ids containing a certain tag
 * @param slideshow: output slideshow to fill
 * @param num_photo_left: output number of photo left to insert in slideshow
 */
bool insert_next_slide(Slide &current_slide, 
                       std::map<PhotoID, Photo>& input_photos, 
                       std::map<TagID,std::set<PhotoID>>& tag_to_photo, 
                       std::vector<Slide> &slideshow, 
                       int &num_photo_left) {
  std::set<TagID> current_slide_tags = current_slide.tags();
  std::vector<std::set<PhotoID>*> photos_with_all_tags;
  for(std::set<TagID>::iterator tag_it = current_slide_tags.begin(); tag_it != current_slide_tags.end(); tag_it++) {
    auto find_tag = tag_to_photo.find(*tag_it);
    if(find_tag != tag_to_photo.end()) {
      photos_with_all_tags.push_back(&(find_tag->second));
    }
  }
  
  // removes photo id duplicates and count number of shared tags
  std::map<PhotoID, int> build_num_shared_tag; //id_photo and num_shared_tag
  for(int i = 0; i < photos_with_all_tags.size(); i++) {
    std::set<PhotoID> *photos_with_tag = photos_with_all_tags[i];
	int n = 5000; // take first 5000 samples for each tag
	std::set<PhotoID>::iterator photo_it = photos_with_tag->begin();
	while (photo_it != photos_with_tag->end() && n > 0) {
		auto find_photo = input_photos.find(*photo_it);
		if (find_photo == input_photos.end() || find_photo->second.is_used()) {
			photo_it++;
			continue;
		}
		auto insert = build_num_shared_tag.emplace(*photo_it, 1);
		if (!(insert.second)) {
			insert.first->second++;
		}
		photo_it++;
		n--;
	}
  }
  
  if(build_num_shared_tag.empty()) {
    // Random slide
    PhotoID random_photo_id = 0;
    for (random_photo_id = 0; random_photo_id < input_photos.size(); random_photo_id++) {
      auto find_photo = input_photos.find(random_photo_id);
      if(find_photo == input_photos.end() || !find_photo->second.is_used()) {
        find_photo->second.set_used(true);
        Slide next_slide(find_photo->second);
        current_slide = next_slide;
        slideshow.push_back(next_slide);
        num_photo_left--;
		for (TagID tag : next_slide.tags()) {
			auto find_tag = tag_to_photo.find(tag);
			find_tag->second.erase(random_photo_id);
		}
        break; 
      }
    }
    return true;
  }

  // generate and sort candidates to look for
  std::vector<std::pair<PhotoID,int>> candidates; //id_photo and score
  int num_tag_current_slide = current_slide.tags().size();
  for(std::map<PhotoID, int>::iterator tag_it = build_num_shared_tag.begin(); tag_it != build_num_shared_tag.end(); tag_it++) {
	int num_tags_candidate = input_photos.at(tag_it->first).tags().size();
	int num_shared_tags = tag_it->second;
	int score = std::min(num_shared_tags, std::min(num_tags_candidate - num_shared_tags, num_tag_current_slide - num_shared_tags));
	candidates.push_back(std::pair<PhotoID,int>(tag_it->first, score));
  }

  std::sort(candidates.begin(), candidates.end(), sort_by_second); 

  // find best candidate
  PhotoID good_photo_id = candidates.back().first;
  
  // add next slide
  auto find_photo = input_photos.find(good_photo_id);
  find_photo->second.set_used(true);
  Slide next_slide(find_photo->second);
  current_slide = next_slide;
  slideshow.push_back(next_slide);
  num_photo_left--;
  for (TagID tag : next_slide.tags()) {
	  auto find_tag = tag_to_photo.find(tag);
	  find_tag->second.erase(good_photo_id);
  }
  
  std::cout << "\r [" << num_photo_left << " / "<< input_photos.size() << "] " << std::flush;
  return true;  
}

int main(int argc, char **argv) {
  
  if(argc != 2) {
    std::cout << "Usage: slideshow input \n";
    return -1;
  }
  std::map<TagID,std::set<PhotoID>> tag_to_photo; 
  std::map<PhotoID, Photo> input_photos;
  read_input(argv[1], input_photos, tag_to_photo);
  std::map<PhotoID, Photo> input_photos_backup = input_photos;
  
  std::vector<Slide> slideshow;
  Slide first_slide(input_photos.begin()->second);
  input_photos.begin()->second.set_used(true);
  slideshow.push_back(first_slide);
  int num_photo_left = input_photos.size() - 1; 
  while (num_photo_left != 0) {
    if (!insert_next_slide(first_slide, input_photos, tag_to_photo, slideshow, num_photo_left)) {
      break;
    }
  }
  std::string output_filename = argv[1]; 
  output_filename = "..\\output" + output_filename.substr(output_filename.find_last_of("\\"), output_filename.size()-1);
  output_filename = output_filename.substr(0, output_filename.find_last_of(".")) + "_submission.txt";  
  write_output(input_photos_backup, slideshow, output_filename);
  return 0;
}