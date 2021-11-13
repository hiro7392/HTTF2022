#include <bits/stdc++.h>

#define rep(i, N) for (int i = 0; i < N; i++)
#define rep2(i, N) for (int i = 1; i <= N; i++)
using namespace std;
long long INF = 1e18;
long long mod = 1e9 + 7;

int dx[4] = {1, 0, -1, 0};
int dy[4] = {0, 1, 0, -1};
int skillNum = 0;
int memberNum = 0;
#define debug 0
#define debugWorkTime 0
#define debugWorkTimeMax 1
#define debugWorkTimeMin 0
#define debugWorkTimeAverage 0
#define sortByMaxMinAve 1

#define debugWorkMember 0
#define useSample 0
#define useTestDate 0
#define setRandonSelect 0
int days = 0;
vector<vector<int>> from(1010);
vector<vector<int>> to(1010);

// taskSkill[i][k]:=タスクiに必要な技能k
double taskSkill[1010][25];

// skillSum[i]:=タスクiの必要な技能の合計値
double skillSum[1010];

// PreDoneTask[i]:=メンバーiが直前にやったタスクにかかった時間とそのタスク番号
vector<pair<double, int>> PreDoneTask(25, make_pair(1e5, -1));
// assing[i]:=メンバーiに割り振られたタスク、タスクがなければ-1を入れる
vector<long long> taskAssign(25);

// assing[i]:=タスクiに割り振られたメンバー、メンバがなければ-1を入れる
vector<long long> memberAssign(1010);

//メンバー　iがやっている仕事にかかる時間
vector<long long> timeUntilFinish(25, INF);

// finished[i]:=タスクiが終了したかどうか.終了していたらtrue
bool finished[1010] = {};

// 進行中かどうか、進行中ならtrue
bool onGoing[1010] = {};


// estimatedSkill[i][p]:=メンバーiのスキルpの推定値
double estimatedSkill[1010][25];

//メンバ-iが現在やっているタスクにかかる時間の推定値
vector<double>estTime(25,3.0);

// 現在そのタスクに取り組むことができるか
bool availableTask[1010];

//現在取り組むことのできるタスクのリスト
vector<int> availableTasks;

// vector<pair<前のタスクにかかった時間,メンバーの番号>>selectOrder
vector<pair<double, int>> selectOrder(25);

//ソート用
vector<pair<double, int>> sortTaskByL2norm;

bool cmp(pair<double, int> p1, pair<double, int> p2) {
  return p1.first < p2.first;
}
bool cmpInv(pair<double, int> p1, pair<double, int> p2) {
  return p1.first > p2.first;
}
//現在のタスクにかかっている時間
vector<int> workingTime(25, 0);

// completedTask[i]:=メンバーiがタスクをクリアした数
vector<int> completedTask(25, 0);

//タスクiが達成されるのにかかった日数と
//その仕事をやった人を記録
// vector<pair<int, double>> completeTimeAndMember(1010);

// finishTimeAndMember[i]:=<かかった日数,タスク番号k>
//メンバ-iがタスク番号kを達成するのにかかった日数
//
vector<multimap<int, int>> finishTimeAndMember(1010);

struct MemberInfo {
  //タスクに要した時間の最大,最小、平均
  double WorkTimeMax;
  double WorkTimeMin;
  double WorkTimeAve;
  double nowWorkTime;

  //コンストラクタ
  MemberInfo() {
    WorkTimeMin = 1e5;
    WorkTimeMax = 0.0;
    WorkTimeAve = 0.0;
    nowWorkTime = -1.0;
  }
  //タスクをアップデート
  void updateInfo() {}

  double getRateValue() {
    return WorkTimeAve * 0.1 + WorkTimeMax * 0.3 + WorkTimeMin * 0.3;
  }
};
//タスクlのL2ノルムを計算する
double calL2norm(int l, int k) {
  double ret = 0.0;
  rep(i, k) ret += (taskSkill[l][i] * taskSkill[l][i]);
  return sqrt(ret);
}

// L2ノルムでソートする
void sortTasks(int N, int K) {
  rep(i, N) sortTaskByL2norm.push_back(make_pair(calL2norm(i, K), i));

  sort(sortTaskByL2norm.begin(), sortTaskByL2norm.end(), cmp);
  return;
}
//
void sortTasksByTo(int N) {
  rep(i, N) sortTaskByL2norm.push_back(make_pair(to[i].size(), i));
  sort(sortTaskByL2norm.begin(), sortTaskByL2norm.end(), cmp);
}
std::random_device rnd;  // 非決定的な乱数生成器を生成
std::mt19937 mt(
    rnd());  //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
//メンバーiがタスクkについてtime時間かかった時の推定
void estSkill(int i, int k, int time) {
  std::uniform_real_distribution<> rand(
      0, skillNum);  // [0.0,skillNum] 範囲の一様乱数
  rep(l, skillNum) {
    if(abs(estTime[i]-time)<5){
      continue;
    }
    else if(abs(estTime[i]-time)>20)estimatedSkill[i][l] = taskSkill[k][l]+rand(mt)/skillNum;
    else{
      estimatedSkill[i][l]=(estimatedSkill[i][l]*completedTask[i]+taskSkill[k][l])/(completedTask[i]+1.0);
    }
  }
  //ランダムに選択した項目iついて、平均から-1.0するする
  rep(l, time/2.0) {
    int skillIndex = rand(mt);
    
    //メンバーiがタスクkについてtime時間かかった時の推定
    estimatedSkill[i][skillIndex] -=  2.0;//3*rand(mt)/skillNum;
    estimatedSkill[i][skillIndex]=max(0.0,estimatedSkill[i][skillIndex]);
  }
  return;
}
//メンバーを格納する配列
vector<MemberInfo> Members(25);

//ソートの比較関数
bool sortMembers(MemberInfo m1, MemberInfo m2) { return true; }

void makeSelectOrder(int m) {
  rep(i, m) selectOrder[i] = make_pair(0.5, i);
  return;
}
//タスクの類似度を降順にmapで管理
vector<multimap<double, int>> taskSimByDown(1010);

//あるメンバーの推定スキル平均
double skillAve[1010];
//タスクxとタスクyの類似度を求める
// 類似度=cos類似度とする
double getSim(int x, int y, int k) {
  double xSumSq = 0.0, ySumSq = 0.0, xySum = 0.0;

  rep(i, k) {
    xSumSq += taskSkill[x][i];
    ySumSq += taskSkill[y][i];
    xySum += taskSkill[x][i] * taskSkill[y][i];
  }
  skillSum[x] = xSumSq;
  skillSum[y] = ySumSq;
  xSumSq = sqrt(xSumSq);
  ySumSq = sqrt(ySumSq);
  return xySum / (xSumSq * ySumSq);
}

//全てのタスク同士の類似度を求める
void calSimilarity(int n, int k) {
  double sim;
  for (int i = 0; i < n; i++) {
    for (int l = 0; l < n; l++) {
      if (i == l) continue;
      sim = getSim(i, l, k);
      //各タスクについて、他のタスクとの類似度をmapに入れておく
      taskSimByDown[i].insert(make_pair(sim, l));
      taskSimByDown[l].insert(make_pair(sim, i));
    }
  }
}

//依存関係の入力と構築
void makeDependency(int R) {
  //依存関係
  rep(i, R) {
    int u, v;
    cin >> u >> v;
    u--, v--;
    //依存関係をtoとfromで表現
    // to[1]=2　の時1->２の依存関係がある
    to[u].push_back(v);
    // from[2]=1の時2->の依存関係がある
    from[v].push_back(u);
  }
}
void makeTasks(int N, int K) {
  // taskに必要な技能
  rep(i, N) {
    rep(l, K) { cin >> taskSkill[i][l]; }
  }
}

//ランダムな0〜nまでの数列を返す
vector<int> getRandomVec(int n) {
  vector<int> shuffle(n);
  rep(i, n) shuffle[i] = i;
  std::random_device seed_gen;
  std::mt19937 engine(seed_gen());
  std::shuffle(shuffle.begin(), shuffle.end(), engine);
  return shuffle;
}

//  タスクkが現在取り組むことができるか判定
//  取り組めるならtrueを返す
bool taskIsReady(int k) {
  //すでに進行中又は終了後だったらダメ
  if (onGoing[k] || finished[k]) return false;

  //依存するタスクが全て終わっているかチェック
  for (auto nx : from[k]) {
    if (!finished[nx]) return false;
  }
  return true;
}

double getMemberSkillSum(int member, int nowTask, double nowTime) {
  int preTask = PreDoneTask[member].second;
  if (preTask < 0) return 1.0;

  double preTime = PreDoneTask[member].first;
  return (skillSum[preTask] + skillSum[nowTask] - (preTime + nowTime)) / 2.0;
}
//  メンバーiの仕事が終わったことを記録
void setFinish(int i) {
  //担当していたタスクの終了を記録
  finished[taskAssign[i]] = true;
  //進行中フラグを閉じる
  // onGoing[taskAssign[i]] = false;
  rep(k, 25) {
    if (selectOrder[k].second == i) {
      //今までの作業にかかった時間の最大値で更新
      // selectOrder[k].first = max(selectOrder[k].first, workingTime[i]);

      //今までの作業にかかった時間の最小値で更新
      if (selectOrder[k].first == 0.0)
        selectOrder[k].first = (double)workingTime[i];
      else {
#if debugWorkTimeMin

        selectOrder[k].first =
            min(selectOrder[k].first, (double)workingTime[i]);
#elif 0  // debugWorkTimeMax

        selectOrder[k].first =
            max(selectOrder[k].first, (double)workingTime[i]);
#elif sortByMaxMinAve
        //最小値
        Members[i].WorkTimeMin =
            min(Members[i].WorkTimeMin, (double)workingTime[i]);
        //平均値
        Members[i].WorkTimeAve =
            (double)(Members[i].WorkTimeAve * completedTask[i] +
                     workingTime[i]) /
            (completedTask[i] + 1.0);
        //最大値
        Members[i].WorkTimeMax =
            max(Members[i].WorkTimeMax, (double)workingTime[i]);
        // selectOrder[k].first = Members[i].getRateValue();
        double skilllSumNow =
            getMemberSkillSum(i, taskAssign[i], workingTime[i]);
        selectOrder[k].first =
            Members[i].getRateValue();  // / (skilllSumNow + 1);
        // printf("# skillSum member %d = %lf\n",i,getMemberSkillSum(i,
        // taskAssign[i], workingTime[i]));
        estSkill(i, taskAssign[i], workingTime[i]);
        // printf("#s %d", i + 1);
        // rep(p, skillNum) printf(" %lf", estimatedSkill[i][p]);
        // printf("\n");
        
        

        PreDoneTask[i] = make_pair((double)workingTime[i], taskAssign[i]);

#if 0
        // debug用
        printf("# member %d (Max %lf Min %lf Ave %lf Rate =%lfM )\n",i, Members[i].WorkTimeMax,
               Members[i].WorkTimeMin, Members[i].WorkTimeAve,
               Members[i].getRateValue());
#endif
#endif
      }
      break;
    }
  }

  //タスクにかかった時間を記録する
  finishTimeAndMember[i].insert(make_pair(workingTime[i], taskAssign[i]));
  //かかった時間を記録
  //現在こなしたタスク数を増やす
  completedTask[i]++;
  //現在の作業時間を0にする
  workingTime[i] = 0;
  //メンバーiに割り当てられたタスクをからにする
  taskAssign[i] = -1;
  return;
}

//空いているタスクがあれば取ってくる、なければfalse
// メンバーi タスク k
bool getTask(int i, int n) {
  for (int l = 0; l < n; l++) {
    int k = l;  //= sortTaskByL2norm[l].second;
    //タスクkが開始可能であれば割り当てる
    if (taskIsReady(k)) {
      taskAssign[i] = k;
      // memberAssign[k] = i;
      onGoing[k] = true;
      return true;
    }
  }
  return false;
}
//空いているメンバーがいれば割り当てる
//　タスクi メンバー数m
bool getMember(int i, int m) {
  rep(k, m) {
    //メンバーkが労働可能であれば割り当てる
    if (taskAssign[k] < 0) {
      taskAssign[k] = i;
      memberAssign[i] = k;
      onGoing[i] = true;
      return true;
    }
  }
  return false;
}

void updateProgress(int M) {
  rep(i, M) {
    if (taskAssign[i] > -1) {
      //残り日数を1日減らす
      timeUntilFinish[i]--;
      //
      if (timeUntilFinish[i] <= 0) setFinish(i);
    }
  }
}

//メンバーの番号が若い順にタスクを割り当てていく
void simpleAssign(int m, int n) {
  //今日作業を開始する人とタスクのペアが入った出力結果
  vector<pair<int, int>> output;

  vector<int> shuffle(m);
  //最初の割り当てをランダムに行うとき
#if setRandonSelect
  shuffle = getRandomVec(notWorkedMembers);
#else
  rep(i, m) shuffle[i] = i;
#endif

  rep(l, m) {
    int i = shuffle[l];
    if ((taskAssign[i] < 0) && getTask(i, n)) {
      output.push_back(make_pair(i, taskAssign[i]));
    }
  }
  //出力用
  printf("%d ", output.size());
  for (auto now : output) {
    printf("%d %d ", now.first + 1, now.second + 1);
  }
  printf("\n");
}

#if 0
//タスクiについて、類似度が高いタスクを早くこなした
//メンバーのindexを返す
int getOptMember(int i) {
  //メンバーiの他のタスクとの類似度が<double,int>のpairで入ったベクトル
  //降順にソートずみ
  // vector<pair<double, int>> vec = getSimVec(i, n);
  double threshold = 10.0;
  auto it = taskSimByDown[i].rbegin();
  int taskIndex;
  //類似度が高い順にクリアした人が空いているかみていく
  while (it != taskSimByDown[i].rend()) {
    taskIndex = it->second;
    if (!finished[taskIndex]) {
      it++;
      continue;
    }
    // <類似度,タスク番号>
    //一定時間よりも短い時間で終わらせていて
    //かつその人が今何もしていなければ割り当てる
    if ((completeTimeAndMember[taskIndex].second < threshold) &&
        (taskAssign[completeTimeAndMember[taskIndex].first] < 0)) {
      return completeTimeAndMember[it->second].first;
    }
    it++;
  }
  return -1;
}
#endif
int getOptTask(int i) {
  if (finishTimeAndMember[i].empty()) return -1;
  //メンバーiの他のタスクとの類似度が<double,int>のpairで入ったベクトル
  //降順にソートずみ
  double threshold = 10.0;

  int taskIndex;
  auto it = finishTimeAndMember[i].begin();
  //メンバーiがやったタスクについてみていく
  // while (it != finishTimeAndMember[i].end()) {
  // if (it->first > 10.0) {
  //   it++;
  //   continue;
  // }
  taskIndex = it->second;
  // task番号==taskIndex のタスクについて類似度の高いものを取ってくる
  auto itSim = taskSimByDown[taskIndex].rbegin();
  while (itSim != taskSimByDown[taskIndex].rend()) {
    //類似度が高いものを返す
    if (taskIsReady(itSim->second)) {
      return itSim->second;
    }
    itSim++;
  }
  it++;
  //}
  return -1;
}
void registTaskToMember(int member, int task) {
  taskAssign[member] = task;
  memberAssign[task] = member;
  onGoing[task] = true;
  return;
}

//現在取り組めるタスクについて
//似たタスクを早く終わらせた人に割り当てる
void assignByTaskSimilarity(int m, int n) {
  vector<pair<int, int>> output;  //出力用ベクトル
  sort(selectOrder.begin(), selectOrder.begin() + m, cmp);

  //空いているメンバーについて割り当てる
  rep(l, m) {
    int i = l;
    // i= selectOrder[l].second;
    if (taskAssign[i] >= 0) continue;

    //メンバーiについて、
    //過去に行ったタスクと類似度が高いタスクを取ってくる
    int taskCand = -1;
    if (days > 500) taskCand = getOptTask(i);
    if (taskCand > -1) {
      registTaskToMember(i, taskCand);
      output.push_back(make_pair(i, taskCand));
    } else {
      if (getTask(i, n)) {
        // registTaskToMember(i, );
        output.push_back(make_pair(i, taskAssign[i]));
        // break;
      }
    }
  }
  printf("%d ", output.size());
  for (auto now : output) {
    printf("%d %d ", now.first + 1, now.second + 1);
  }
  printf("\n");
  return;
}
double getDiffSum(int i, int l, int k) {
  double ret = 0.0;
  rep(p, k) ret += max(0.0, taskSkill[l][p] - estimatedSkill[i][p]);
  return ret;
}

//誤差が最も小さいものを取ってくる
pair<int, double> getTaskByDiff(int i, int k, int n) {
  double minDiff = 1e5;
  int retTask = -1;
  rep(l, n) {
    if (taskIsReady(l)) {
      // if (skillAve[i] = 0.0) return l;
      double nowDiff = getDiffSum(i, l, k);
      if (nowDiff < minDiff) {
        retTask = l;
        minDiff = nowDiff;
      }
    }
  }
  return make_pair(retTask, minDiff);
}

//メンバーのスキルの推定値との差が最も小さいタスクを割り当てる
void assignByTaskDiff(int m, int n) {
  vector<pair<int, int>> output;  //出力用ベクトル
  sort(selectOrder.begin(), selectOrder.begin() + m, cmp);

  //空いているメンバーについて割り当てる
  rep(l, m) {
    int i = l;
    i = selectOrder[l].second;
    if (taskAssign[i] >= 0) continue;
    //メンバーiについて、
    int nextTask = -1;
    double cost = 1e5;
    //3回やって最も良い時を採用
    rep(_, 1) {
      //estSkill(i, PreDoneTask[i].second, PreDoneTask[i].first);
      auto taskAndDay = getTaskByDiff(i, skillNum, n);
      if (taskAndDay.second < cost) {
        cost = taskAndDay.second;
        nextTask = taskAndDay.first;
      }
    }
    if (nextTask > -1) {
      estTime[i]=cost;
      registTaskToMember(i, nextTask);
      output.push_back(make_pair(i, taskAssign[i]));
      // break;
    }
  }
  printf("%d ", output.size());
  for (auto now : output) {
    printf("%d %d ", now.first + 1, now.second + 1);
  }
  printf("\n");
  return;
}

//メンバーiが今日働いた分を記録
void registWorkTime(int m) {
  rep(i, m) {
    if (taskAssign[i] != -1) {
      workingTime[i]++;
    }
  }
  return;
}

//デバッグ用　全員のタスク割り当てを出力
void printNowAssignMent(int m) {
  rep(i, m) {
    int taskStatus = taskAssign[i];
    if (taskStatus == -1) {
      printf("#メンバー %d の　進行中のタスクはありません\n", i + 1);
    } else {
      printf("#メンバー %d の　進行中のタスクは　タスク%d です\n", i + 1,
             taskStatus + 1);
    }
  }
}

//全てのタスクが終わっているか確認する
bool allTasksCompleted(int n) {
  rep(i, n) {
    if (!finished[i]) return false;
  }
  return true;
}

//現在取り組めるタスクを更新
void updateAvalableTask(int n) {
  // printf("#update Availtask called!\n");
  rep(i, n) {
    if (availableTask[i]) {
      continue;
    } else if (taskIsReady(i)) {
      //すでに追加したものはもう追加しない
      availableTask[i] = true;
      availableTasks.push_back(i);
    }
  }
}

int main() {
  // clock_t start = clock();  // スタート時間
  long long N, M, K, R;
  cin >> N >> M >> K >> R;

  //タスク関係の入力
  makeTasks(N, K);
  //依存関係の入力と構築
  makeDependency(R);
  //メンバーを整列させて初期化
  makeSelectOrder(M);
  //タスク間の類似度を計算
  calSimilarity(N, K);

  int status = -1;
  rep(i, M) taskAssign[i] = -1;
#if useTestDate
  rep(i, M) {
    rep(k, K) {
      int skill;
      cin >> skill;
    }
  }
  rep(i, N) {
    rep(l, M) {
      int skill;
      cin >> timeToNeed[l][i];
    }
  }
#endif
  rep(i, N) {
    finished[i] = false;
    // availableTask[i] = false;
  }

  //初日に取り組めるタスクを更新
  // updateAvalableTask(N);

  //タスクをL2ノルムが小さい順にソート
  // sortTasks(N, K);

  // sortTasksByTo(N);
  //  //初日の割り当て
  // assignByTaskSimilarity(M, N);
  assignByTaskDiff(M, N);

  skillNum = K;
  memberNum = M;
  while (1) {
    days++;
    //メンバーの労働時間を記録
    registWorkTime(M);
    //今日の分の進み具合を更新
#if useSample
    updateProgress(M);
    if (allTasksCompleted(N) || days > 2000) break;
#else
    //今日仕事を終えたメンバーを記録
    cin >> status;
    if (status < 0) break;
    rep(i, status) {
      int finishedMember;
      cin >> finishedMember;
      finishedMember--;
      setFinish(finishedMember);
    }
    //取り組めるタスクのリストを更新
    // if (status > 0) updateAvalableTask(N);
#endif
#if debug
    printf("0 now status =%d\n", status);
#endif
    //メンバーの仕事が早い順に割り当てていく方法
    // assignByFastMember(M, N);

    //何も考えずに番号が若い順にタスクを割り当てる方法
    // simpleAssign(M, N);

    //タスクをタスク間の類似度に基づいて分類
    // assignByTaskSimilarity(M, N);

    assignByTaskDiff(M, N);
  }
  // clock_t end = clock();  // 終了時間
  // printf("#duration = %lf\n", (double)(end - start) / CLOCKS_PER_SEC);
#if 0  // debug
  rep(i, N) {
    printf("#task%d was done by member%din%lfdays\n", i,
          completeTimeAndMember[i].first, completeTimeAndMember[i].second);
  }
#endif
}
/*
3 2 2 2
0 1
2 0
1 1
1 2
2 3

*/
