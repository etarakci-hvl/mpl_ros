#include <planner/mp_map_util.h>

MPMapUtil::MPMapUtil(bool verbose) :
  planner_verbose_(verbose)
{
 if(planner_verbose_)
    printf(ANSI_COLOR_CYAN "PLANNER VERBOSE ON\n" ANSI_COLOR_RESET);
}

std::vector<Waypoint> MPMapUtil::getPath() {
  return path_;
}

void MPMapUtil::setEpsilon(decimal_t eps) {
  epsilon_ = eps;
}

void MPMapUtil::setDt(decimal_t dt) {
  ENV_->set_dt(dt);
  ENV_->set_discretization(false);
}

void MPMapUtil::setAmax(decimal_t a_max) {
  ENV_->set_a_max(a_max);
}

void MPMapUtil::setVmax(decimal_t v_max) {
  ENV_->set_v_max(v_max);
}

void MPMapUtil::setMapUtil(std::shared_ptr<VoxelMapUtil> map_util) {
  ENV_.reset(new mrsl::env_map(map_util));
}

bool MPMapUtil::plan(const Waypoint &start, const Waypoint &goal) {
  path_.clear();
  primitives_.clear();

  if(planner_verbose_) {
    printf("start pos: [%f, %f, %f], vel: [%f, %f, %f], acc: [%f, %f, %f]\n",
        start.pos(0), start.pos(1), start.pos(2),
        start.vel(0), start.vel(1), start.vel(2),
        start.acc(0), start.acc(1), start.acc(2));
    printf("goal pos: [%f, %f, %f], vel: [%f, %f, %f], acc: [%f, %f, %f]\n",
        goal.pos(0), goal.pos(1), goal.pos(2),
        goal.vel(0), goal.vel(1), goal.vel(2),
        goal.acc(0), goal.acc(1), goal.acc(2));
  }

  mrsl::ARAStar<Waypoint> AA;
  std::vector<int> action_idx;
  std::list<Waypoint> path;

  //ENV_->reset();
  ENV_->set_goal(goal);
  double pcost = AA.Astar(start, ENV_->state_to_idx(start), *ENV_, path, action_idx, epsilon_);

  std::cout << "Plan cost = " << pcost << std::endl;
  std::cout << "Path length = " << path.size() << std::endl;
  std::cout << "action_idx.size() = " << action_idx.size() << std::endl;


  //ps_ = ENV_->ps_;
  if (path.empty()) {
    if(planner_verbose_)
      printf(ANSI_COLOR_RED "Cannot find a path, Abort!" ANSI_COLOR_RESET "\n");
    return false;
  }

  path_.push_back(start);
  for (const auto &it_node : path)
    path_.push_back(it_node);

  //std::reverse(path_.begin(), path_.end());
  return true;
}

Trajectory MPMapUtil::getTraj() {
  std::vector<Primitive> ps;

  for(int i = 0; i < (int)path_.size()-1; i++){
    Waypoint nw1 = path_[i];
    Waypoint nw2 = path_[i+1];

    nw1.use_pos = true;
    nw1.use_vel = true;
    nw1.use_acc = false;

    nw2.use_pos = true;
    nw2.use_vel = true;
    nw2.use_acc = false;

    Primitive p(nw1, nw2, ENV_->get_dt());
    ps.push_back(p);
  }

  return Trajectory(ps);
}
