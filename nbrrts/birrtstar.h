#ifndef BIRRTSTAR
#define BIRRTSTAR

#include <rrtstartree.h>
#include <QDebug>

namespace RRTStar
{
    /**
     * @brief Bi-directional RRT
     * @details It is often preferable to use two RRTs when searching the state space with
     * one rooted at the source and one rooted at the goal.  When the two trees intersect,
     * a solution has been found.
     */
    template<typename T>
    class BiRRTStar {
    public:
        BiRRTStar(std::shared_ptr<StateSpace<T>> stateSpace) : _startTree(stateSpace), _goalTree(stateSpace) {
            reset();
        }

        void reset() {
            _startTree.reset();
            _goalTree.reset();

            _iterationCount = 0;

            _startSolutionNode = nullptr;
            _goalSolutionNode = nullptr;
            _solutionLength = DBL_MAX;
        }
        
        void initVector(int num, int initW, int initH) {
            _startTree.initVector(num, initW, initH);
            _goalTree.initVector(num, initW, initH);
        }


        const rrtStarTree<T> &startTree() const {
            return _startTree;
        }
        const rrtStarTree<T> &goalTree() const {
            return _goalTree;
        }

        bool isASCEnabled() const {
            return _startTree.isASCEnabled();
        }
        void setASCEnabled(bool checked) {
            _startTree.setASCEnabled(checked);
            _goalTree.setASCEnabled(checked);
        }

        float goalBias() const {
            return _startTree.goalBias();
        }
        void setGoalBias(float goalBias) {
            _startTree.setGoalBias(goalBias);
            _goalTree.setGoalBias(goalBias);
        }

        int maxIterations() const {
            return _startTree.maxIterations();
        }
        void setMaxIterations(int itr) {
            _startTree.setMaxIterations(itr);
            _goalTree.setMaxIterations(itr);
        }

        float waypointBias() const {
            return _startTree.waypointBias();
        }
        void setWaypointBias(float waypointBias) {
            _startTree.setWaypointBias(waypointBias);
            _goalTree.setWaypointBias(waypointBias);
        }

        const std::vector<T> &waypoints() {
            return _startTree.waypoints();
        }
        void setWaypoints(const std::vector<T> &waypoints) {
            _startTree.setWaypoints(waypoints);
            _goalTree.setWaypoints(waypoints);
        }

        float stepSize() const {
            return _startTree.stepSize();
        }
        void setStepSize(float stepSize) {
            _startTree.setStepSize(stepSize);
            _goalTree.setStepSize(stepSize);
        }

        float maxStepSize() const {
            return _startTree.maxStepSize();
        }
        void setMaxStepSize(float stepSize) {
            _startTree.setMaxStepSize(stepSize);
            _goalTree.setMaxStepSize(stepSize);
        }

        const double getsolutionLength()
        {
            return _solutionLength;
        }

        float goalMaxDist() const {
            return _startTree.goalMaxDist();
        }
        void setGoalMaxDist(float maxDist) {
            _startTree.setGoalMaxDist(maxDist);
            _goalTree.setGoalMaxDist(maxDist);
        }


        /**
         * @brief Get the shortest path from the start to the goal
         *
         * @param vecOut The vector to place the solution in
         */
        void getPath(std::vector<T> &vecOut) {
            _startTree.getPath(vecOut, _startSolutionNode);
            _goalTree.getPath(vecOut, _goalSolutionNode, true);
        }


        /**
         * @brief
         * @details Attempts to add a new node to each of the two trees.  If
         * a new solution is found that is shorter than any previous solution, we store
         * it instead.
         */
        /**/
        void grow() {
            double depth;
            Node<T> *otherNode;

            Node<T> *newStartNode = _startTree.grow(_solutionLength);
            if (newStartNode) {
                otherNode = _findBestPath(newStartNode->state(), _goalTree, &depth);
                if (otherNode && ((depth + newStartNode->getDistance()) < _solutionLength)) {
                    _startSolutionNode = newStartNode;
                    _goalSolutionNode = otherNode;
                    _solutionLength = newStartNode->getDistance() + depth;
                }
                _startTree.nearnodes(newStartNode->state());
                _startTree.OptimizeNearNodes(newStartNode,newStartNode->parent());
            }


            Node<T> *newGoalNode = _goalTree.grow(_solutionLength);
            if (newGoalNode) {
                otherNode = _findBestPath(newGoalNode->state(), _startTree, &depth);
                if (otherNode && ((depth + newGoalNode->getDistance()) < _solutionLength)) {
                    _startSolutionNode = otherNode;
                    _goalSolutionNode = newGoalNode;
                    _solutionLength = newGoalNode->getDistance() + depth;
                }
                _goalTree.nearnodes(newGoalNode->state());
                _goalTree.OptimizeNearNodes(newGoalNode,newGoalNode->parent());
            }
            ++_iterationCount;
        }
        

        /**
         * @brief Grows the trees until we find a solution or run out of iterations.
         */
        bool run() {
            for (int i = 0; i < _startTree.maxIterations(); i++) {
                grow();
                if (_startSolutionNode != nullptr) return true;
            }
            return false;
        }


        bool RunToAimLength(double length ,int Exiteration)
        {
            while( this->getsolutionLength () > length)
            {
                this->grow ();
                if(iterationCount () > Exiteration)
                    return false;
            }
            return true;
        }


        void setStartState(const T &start) {
            _startTree.setStartState(start);
            _goalTree.setGoalState(start);
        }
        const T &startState() const {
            return _startTree.startState();
        }

        void setGoalState(const T &goal) {
            _startTree.setGoalState(goal);
            _goalTree.setStartState(goal);
        }
        const T &goalState() const {
            return _startTree.goalState();
        }


        const Node<T> *startSolutionNode() {
            return _startSolutionNode;
        }

        const Node<T> *goalSolutionNode() {
            return _goalSolutionNode;
        }


        int iterationCount() const {
            return _iterationCount;
        }
        
        void print_num_r() {
            qDebug()<<_startTree.getNum_r()<<" "<<_goalTree.getNum_r();
        }


    protected:
        Node<T> *_findBestPath(const T &targetState, rrtStarTree<T> &treeToSearch, double *depthOut) {
            Node<T> *bestNode = nullptr;
            double depth = DBL_MAX;
            
            //int dst = (_goalTree.stateSpace()).getLocationDest(targetState, 100, 100);
            for (Node<T> *other : treeToSearch.allNodes()) {
            //for (Node<T> *other : *(treeToSearch.classifyNodes() + dst)) {
                float dist = _startTree.stateSpace().distance(other->state(), targetState);
                if (dist < goalMaxDist() && other->getDistance() < depth ) {
                    if(_startTree.stateSpace ().transitionValid (other->state (), targetState))
                        if (dist < goalMaxDist() && other->getDistance() < depth )
                        {
                            bestNode = other;
                            depth = other->getDistance() + dist;
                        }
                }
            }

            if (depthOut) *depthOut = depth;

            return bestNode;
        }


    private:
        rrtStarTree<T> _startTree;
        rrtStarTree<T> _goalTree;

        int _iterationCount;

        double _solutionLength;
        Node<T> *_startSolutionNode, *_goalSolutionNode;
    };

};


#endif // BIRRTSTAR

