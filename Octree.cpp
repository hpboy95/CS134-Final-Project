
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Simple Octree Implementation 11/10/2020
// 
//  Copyright (c) by Kevin M. Smith
//  Copying or use without permission is prohibited by law. 
//


#include "Octree.h"
 


//draw a box from a "Box" class  
//
void Octree::drawBox(const Box &box) {
	ofNoFill();
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box Octree::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	cout << "vertices: " << n << endl;
//	cout << "min: " << min << "max: " << max << endl;
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}

// getMeshPointsInBox:  return an array of indices to points in mesh that are contained 
//                      inside the Box.  Return count of points found;
//
int Octree::getMeshPointsInBox(const ofMesh & mesh, const vector<int>& points,
	Box & box, vector<int> & pointsRtn)
{
	int count = 0;
	for (int i = 0; i < points.size(); i++) {
		ofVec3f v = mesh.getVertex(points[i]);
		if (box.inside(Vector3(v.x, v.y, v.z))) {
			count++;
			pointsRtn.push_back(points[i]);
		}
	}
	return count;
}

// getMeshFacesInBox:  return an array of indices to Faces in mesh that are contained 
//                      inside the Box.  Return count of faces found;
//
int Octree::getMeshFacesInBox(const ofMesh & mesh, const vector<int>& faces,
	Box & box, vector<int> & facesRtn)
{
	int count = 0;
	for (int i = 0; i < faces.size(); i++) {
		ofMeshFace face = mesh.getFace(faces[i]);
		ofVec3f v[3];
		v[0] = face.getVertex(0);
		v[1] = face.getVertex(1);
		v[2] = face.getVertex(2);
		Vector3 p[3];
		p[0] = Vector3(v[0].x, v[0].y, v[0].z);
		p[1] = Vector3(v[1].x, v[1].y, v[1].z);
		p[2] = Vector3(v[2].x, v[2].y, v[2].z);
		if (box.inside(p,3)) {
			count++;
			facesRtn.push_back(faces[i]);
		}
	}
	return count;
}

//  Subdivide a Box into eight(8) equal size boxes, return them in boxList;
//
void Octree::subDivideBox8(const Box &box, vector<Box> & boxList) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	float xdist = (max.x() - min.x()) / 2;
	float ydist = (max.y() - min.y()) / 2;
	float zdist = (max.z() - min.z()) / 2;
	Vector3 h = Vector3(0, ydist, 0);

	//  generate ground floor
	//
	Box b[8];
	b[0] = Box(min, center);
	b[1] = Box(b[0].min() + Vector3(xdist, 0, 0), b[0].max() + Vector3(xdist, 0, 0));
	b[2] = Box(b[1].min() + Vector3(0, 0, zdist), b[1].max() + Vector3(0, 0, zdist));
	b[3] = Box(b[2].min() + Vector3(-xdist, 0, 0), b[2].max() + Vector3(-xdist, 0, 0));

	boxList.clear();
	for (int i = 0; i < 4; i++)
		boxList.push_back(b[i]);

	// generate second story
	//
	for (int i = 4; i < 8; i++) {
		b[i] = Box(b[i - 4].min() + h, b[i - 4].max() + h);
		boxList.push_back(b[i]);
	}
}

void Octree::create(const ofMesh & geo, int numLevels) {
	cout << "Creating Octree" << endl;
	float startTime = ofGetElapsedTimeMillis();
	// initialize octree structure
	//
	mesh = geo;
	int level = 0;
	root.box = meshBounds(mesh);
	if (!bUseFaces) {
		for (int i = 0; i < mesh.getNumVertices(); i++) {
			root.points.push_back(i);
		}
	}
	else {
		// need to load face vertices here
		//
	}

	// recursively buid octree
	//
	level++;
    subdivide(mesh, root, numLevels, level);
	float finishTime = (ofGetElapsedTimeMillis() - startTime) / 1000;
	cout << "Finished Octree" << endl;
	cout << "Octree took " << finishTime << " seconds to complete" << endl;
}


//
// subdivide:  recursive function to perform octree subdivision on a mesh
//
//  subdivide(node) algorithm:
//     1) subdivide box in node into 8 equal side boxes - see helper function subDivideBox8().
//     2) For each child box
//            sort point data into each box  (see helper function getMeshFacesInBox())
//        if a child box contains at list 1 point
//            add child to tree
//            if child is not a leaf node (contains more than 1 point)
//               recursively call subdivide(child)
//         
//      
             
void Octree::subdivide(const ofMesh & mesh, TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	// subdvide algorithm implemented here
	//1) subdivide
	vector<Box> children;
	subDivideBox8(node.box, children);
	for (int i = 0; i < children.size(); i++) { //go though all the children
		vector<int> pointsReturn;
		getMeshPointsInBox(mesh, node.points, children[i], pointsReturn); //This will set all the points in the box in the node
		if (pointsReturn.size() > 0) { //Child contains points
			TreeNode &t = TreeNode();
			t.box = children[i]; //Set the node box
			t.points = pointsReturn;
			node.children.push_back(t); //Add to current node's children
			if (pointsReturn.size() > 1) { //Child is not leaf
				subdivide(mesh, node.children.back(), numLevels, level + 1); //recursively subdivide
			}
			else {
				numLeaf++;
			}
		}
	}
}

// Implement functions below for Homework project
//

/*
* Ray: The ray to check for intersection
* Node: The current tree node
* nodeRtn: The intersected tree node
*/
//bool Octree::intersect(const Ray &ray, const TreeNode & node, TreeNode & nodeRtn) {
//	bool intersects = false;
//	//Go through all the children and get the closest intersecting one
//	if (node.children.size() > 0) {
//		int closest = -1;
//		float closestDistance = FLT_MAX;
//		for (int i = 0; i < node.children.size(); i++) { //go though all the children
//			bool intersects = node.children[i].box.intersect(ray, -10000, 10000);
//			if (intersects) {
//				//It intersects check if it is closer than previous and set it if it is
//				glm::vec3 rayOrigin = glm::vec3(ray.origin.x(), ray.origin.y(), ray.origin.z());
//				Vector3 precenter = (node.children[i].box.parameters[1] - node.children[i].box.parameters[0]) / 2 + node.children[i].box.parameters[0]; //Center function
//				glm::vec3 center = glm::vec3(precenter.x(), precenter.y(), precenter.z());
//				glm::max(glm::vec3(glm::abs(rayOrigin) - center), glm::vec3(0));
//				glm::vec3 q = glm::vec3(glm::abs(rayOrigin) - center);
//				float distance = glm::max(q, glm::vec3(0)).length() + glm::min(0.0f, glm::max(q.x, max(q.y, q.z))); //Box SDF
//				if (distance < closestDistance) { //Always choose only the closest one
//					closestDistance = distance;
//					closest = i;
//				}
//			}
//		}
//		if (closest < 0) {
//			//Worst Case no intersections found inside
//			return false;
//		}
//		intersects = intersect(ray, node.children[closest], nodeRtn);
//	}
//	else {
//		nodeRtn = node; //There are no children This one has to be it
//		intersects = true;
//	}
//	return intersects;
//}

bool Octree::intersect(const Ray & ray, const TreeNode & node, TreeNode & nodeRtn) {
	bool inter = false;
	if (node.points.size() == 1) {
		inter = node.box.intersect(ray, -10000, 10000);
		if (inter)
		{
			nodeRtn = node;
		}
	}
	else {
		for (int i = 0; i < node.children.size(); i++) { //go though all the children
			if (node.children[i].box.intersect(ray, -10000, 10000) && intersect(ray, node.children[i], nodeRtn)) { //Child overlaps
				inter = true;
				break;
			}
		}
	}	
	return inter;
}

bool Octree::intersect(const Box &box, TreeNode & node, vector<Box> & boxListRtn) {
	bool overlap = false;
	if (node.children.size() == 0) {
		boxListRtn.push_back(node.box);
	}
	else {
		for (int i = 0; i < node.children.size(); i++) { //go though all the children
			bool overlap = node.children[i].box.overlap(box); //This will set all the points in the box in the node
			if (overlap) { //Child overlaps
				intersect(box, node.children[i], boxListRtn);
			}
		}
	}
	if (boxListRtn.size() > 1) {
		return true;
	}
	return false;
}

void Octree::draw(TreeNode & node, int numLevels, int level) {
	if (level >= numLevels) return;
	//cout << "level: " << level << endl;
	//cout << "number of points " << node.points.size() << endl;
	ofSetColor(colors[level]);
	drawBox(node.box);
	level++;
	for (int i = 0; i < node.children.size(); i++) {
		draw(node.children[i], numLevels, level);
	}
}

// Optional
//
void Octree::drawLeafNodes(TreeNode & node) {
	if (node.children.size() > 0) {
		for (int i = 0; i < node.children.size(); i++) {
			drawLeafNodes(node.children[i]);
		}
	}
	else {
		ofSetColor(ofColor::white);
		drawBox(node.box);
	}
}




