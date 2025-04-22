#include <iostream>
#include<fstream>
#include<vector>
#include<queue>
#include<string>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include<random>
#include<algorithm>
#include<exception>
#include <sstream>
#include<set>

using namespace sf;
using namespace std;

string input_filename = "input2.txt";
string output_filename = "output.txt";
ifstream fin;
ofstream fout;
int N, sz;
int T;

class Edge {
public:

    double c; // пропускная способность дуги
    double flow; // поток из данной вершины в вершину v

    Edge(double C, double Flow) : c(C), flow(Flow) {

    }

    //конструктор по умолчанию
    Edge()
    {
        c = flow = 0;
    }
};

vector<vector<vector<double>>> simplegraph;
vector<vector<Edge>> graph;
map<int, set<int>> vertextimescanfragmentation;

vector<pair<vector<int>, double>> min_razrez;
vector<int> managed_vertexes;
vector<int> not_managed_vertexes;

// создание простого графа
void CreateSimpleGraph(int N) {
    graph = vector<vector<Edge>>(N - 1, vector<Edge>(N));

    int k, vtx;
    string c;
    for (int u = 0; u < N - 1; u++) {
        fin >> k;
        for (int v = 0; v < k; v++) {
            fin >> vtx >> c;
            graph[u][vtx] = Edge(stod(c.substr(1,c.size() - 2)), 0);
        }
    }
}

//создание временной развёртки графа
void CreateGraphTimeScan(int N, int T) {
    graph = vector<vector<Edge>>(T * N + 1, vector<Edge>(T * N + 2));
    simplegraph = vector<vector<vector<double>>>(N - 1, vector<vector<double>>(N));

    for (int i = 1; i <= T; i++) {
        graph[0][i] = Edge(DBL_MAX, 0);
    }

    int k, vtx;
    string c_arr, cc;
    for (int u = 0; u < N - 1; u++) {
        fin >> k;
        for (int v = 0; v < k; v++) {
            fin >> vtx >> c_arr;
            auto c = c_arr.substr(1, c_arr.size() - 2);
            stringstream c_str(c);
            for (int i = 0; i < T; i++) {
                getline(c_str, cc, ',');
                simplegraph[u][vtx].push_back(stod(cc));
                graph[T * u + i + 1][T * vtx + 1 + (i + 1) % T] = Edge(stod(cc), 0);
                vertextimescanfragmentation[u].insert(T * u + i + 1);
            }
        }
    }

    for (int i = 1; i <= T; i++) {
        graph[T * (N - 1) + i][T * N + 1] = Edge(DBL_MAX, 0);
    }
}

//создание графа
void Create_Graph()
{
    fin.open(input_filename);

    fin >> N >> T;
    if (T == 1)
        CreateSimpleGraph(N);
    else
        CreateGraphTimeScan(N, T);

    fin.close();

    sz = graph.size() + 1;
}

double min(double a, double b)
{
    return a < b ? a : b;
}

//вычисляет максимальный поток и находит минимальный разрез сети
double MaxFlow()
{
    double res = 0;

    vector<int> color(sz);
    vector<vector<double>> flows = vector<vector<double>>(sz, vector<double>{0, 0, 0});
    flows[0][2] = DBL_MAX;

    queue<int> q;
    q.push(0);

    while (!q.empty())
    {
        bool find_flow = false;

        auto u = q.front();
        q.pop();
        color[u] = 1;

        for (int v = 1; v < sz; v++) {

            if (!color[v] && graph[u][v].flow < graph[u][v].c) {

                q.push(v);
                color[v] = 1;
                flows[v] = vector<double>{ double(u), 1, min(graph[u][v].c - graph[u][v].flow,flows[u][2]) };

                if (v == sz - 1) {

                    auto flow = min(graph[u][v].c - graph[u][v].flow, flows[u][2]);
                    res += flow;

                    auto vv = v;
                    while (vv != 0) {
                        if (flows[vv][1] == 1) {
                            graph[u][vv].flow += flow;
                        }
                        else {
                            graph[vv][u].flow -= flow;
                        }
                        vv = u;
                        u = flows[u][0];
                    }

                    flows = vector<vector<double>>(sz, vector<double>{0, 0, 0});
                    flows[0][2] = DBL_MAX;
                    color = vector<int>(sz);
                    q = queue<int>{};
                    q.push(0);
                    find_flow = true;
                }
            }
            else if (v != sz - 1 && !color[v] && graph[v][u].flow)
            {
                q.push(v);
                color[v] = 1;
                flows[v] = vector<double>{ double(u), -1, min(graph[v][u].flow,flows[u][2]) };
            }
        }

        if (find_flow)
            continue;
        else
            color[u] = 2;
    }

    double sum = 0;
    for (int u = 0; u < sz - 1; u++) {
        for (int v = 0; v < sz; v++) {
            if (graph[u][v].flow == graph[u][v].c && graph[u][v].c && sum + graph[u][v].flow <= res) {
                min_razrez.push_back(pair < vector<int>, double >{vector<int>{u, v}, graph[u][v].flow});
                sum += graph[u][v].flow;
            }
        }
    }

    //запись графа после прохождения потока
    fout.open(output_filename, ios_base::app);
    fout << endl;
    for (int i = 0; i < sz - 1; i++) {
        fout << i << " : ";
        for (int j = 0; j < sz; j++)
            if (graph[i][j].c > 0)
                fout << j << " {" << graph[i][j].c << ";" << graph[i][j].flow << "}; ";
        fout << endl;
    }

    fout.close();

    return res;
}

//печать графа
void print_graph()
{
    for (int u = 0; u < sz - 1; u++) {
        cout << u << " : ";
        for (int v = 0; v < sz; v++) {
            cout << "( " << v << " ; " << graph[u][v].c << " ; " << graph[u][v].flow << " ) ";
        }
        cout << endl;
    }
}

void print_min_razrez()
{
    for (auto e : min_razrez) {
        cout << "(" << e.first[0] << " , " << e.first[1] << ") : " << e.second << endl;
    }
    cout << endl;
}

//остановка потока в сети
void StopFlow()
{
    for (int i = 0; i < sz - 1; i++)
        for (int j = 0; j < sz; j++)
            graph[i][j].flow = 0;

    min_razrez.resize(0);
}

string deleteUnImportantZeros(string s) {
    auto count = 0;
    for (int i = s.size() - 1; i >= 0; i--) {
        if (s[i] == '0')
            count++;
        else {
            if (s[i] == '.' || s[i] == ',')
                count++;
            break;
        }
    }
    return s.substr(0, s.size() - count);
}

//определяет, можно ли уменьшить максимальный поток в сети на заданное значение d
double PossibleToLowMaxFlow(double d, double maxflow)
{
    //удаляем управляемые вершины из сети и инцидентные им дуги (присваиваем дугам пропускные способности 0)
    for (auto v : managed_vertexes)
    {
        for (int i = 0; i < sz - 1; i++)
            graph[i][v].c = 0;
        for (int i = 0; i < sz; i++)
            graph[v][i].c = 0;
    }

    //находим новый максимальный поток
    auto new_max_flow = MaxFlow();

    //выводим результат и сеть
    cout << "\nНовый макс. поток = " << new_max_flow << endl;
    print_graph();
    cout << "\nНовый мин. разрез :" << endl;
    print_min_razrez();

    //делаем вывод
    if(managed_vertexes.size() < 2)
        cout << "Управляя вершиной ";
    else
        cout << "Управляя вершинами ";
    for (auto v : managed_vertexes)
        cout << v << ", ";
    if (maxflow - new_max_flow >= d)
        cout << "на значение " << d << " можно уменьшить максимальный поток в сети." << endl;
    else
        cout << "на значение " << d << " нельзя уменьшить максимальный поток в сети." << endl;

    return new_max_flow;
}

int main()
{
    //setlocale(LC_ALL, "russian");

    locale::global(locale("en_US.UTF-8"));

    const wchar_t* window1Name = T == 1 ? L"Исходная сеть" : L"Временная развёртка исходной сети";
    RenderWindow window1(VideoMode(1400, 800), window1Name, Style::Default);
    window1.setVerticalSyncEnabled(true);

    double maxflow = 0, new_max_flow = 0;
    double d = -1;
    int k = 0;
    int x, y;
    Font font;
    font.loadFromFile("arial.ttf");
    srand(time(NULL));

    while (window1.isOpen())
    {
        Event event1;
        while (window1.pollEvent(event1))
        {
            if (event1.type == Event::Closed)
                window1.close();
        }

        //----------Работа алгоритма----------------

        if (k == 0)
        {
            k = 1;

            Create_Graph();

            setlocale(LC_ALL, "russian");
            cout << "Граф:\n" << endl;
            //locale::global(locale("en_US.UTF-8"));
            print_graph();
            maxflow = MaxFlow();
            cout << "\nМаксимальный поток = " << maxflow << endl;
            print_graph();
            cout << "\nМинимальный разрез :" << endl;
            print_min_razrez();

            //------- Визуализация 1.0 (сам граф, если T > 1) -------
            RenderWindow window0(VideoMode(1400, 800), L"Исходная сеть", Style::Default);
            if (T > 1) {
                window0.setVerticalSyncEnabled(true);

                vector<CircleShape> draw_vertices = vector<CircleShape>();

                for (int i = 0; i < N; i++)
                {
                    CircleShape draw_vertex(25);
                    x = rand() % 1350;
                    y = rand() % 750;
                    draw_vertex.setPosition(x, y);
                    draw_vertex.setFillColor(Color::Red);
                    draw_vertices.push_back(draw_vertex);
                }

                window0.clear(Color::Black);

                VertexArray lines(LinesStrip, 0);
                int m = 0;
                for (int i = 0; i < N - 1; i++)
                {
                    for (int j = 0; j < N; j++)
                    {
                        if (simplegraph[i][j].size() > 0) {
                            lines.append(Vertex(Vector2f(draw_vertices[i].getPosition().x + 12.5, draw_vertices[i].getPosition().y + 12.5)));
                            lines.append(Vertex(Vector2f(draw_vertices[j].getPosition().x + 12.5, draw_vertices[j].getPosition().y + 12.5)));
                            lines[m].color = Color::White;
                            lines[m + 1].color = Color::White;

                            Text draw_c_f;
                            draw_c_f.setFont(font);
                            draw_c_f.setCharacterSize(20);

                            string s = "{ ";
                            for (int e = 0; e < simplegraph[i][j].size(); e++) {
                                if (simplegraph[i][j][e] > 0)
                                {
                                    s += deleteUnImportantZeros(to_string(simplegraph[i][j][e])) + " ; ";
                                }
                            }
                            s = s.substr(0, s.size() - 3);
                            s += " }";

                            draw_c_f.setString(s);

                            draw_c_f.setFillColor(Color::Yellow);
                            draw_c_f.setPosition((draw_vertices[i].getPosition().x + 12.5 + draw_vertices[j].getPosition().x + 12.5) / 2.0, (draw_vertices[i].getPosition().y + 12.5 + draw_vertices[j].getPosition().y + 12.5) / 2.0);
                            draw_c_f.setStyle(sf::Text::Bold);
                            window0.draw(draw_c_f);

                            m += 2;
                        }
                    }
                }
                m -= 2;

                for (int i = 0; i < draw_vertices.size(); i++)
                {
                    window0.draw(draw_vertices[i]);
                    Text draw_vertex_number;
                    draw_vertex_number.setFont(font);
                    draw_vertex_number.setCharacterSize(25);
                    draw_vertex_number.setString(to_string(i));
                    draw_vertex_number.setFillColor(Color::Yellow);
                    draw_vertex_number.setPosition(draw_vertices[i].getPosition().x + 12.5, draw_vertices[i].getPosition().y + 12.5);
                    draw_vertex_number.setStyle(sf::Text::Bold);
                    window0.draw(draw_vertex_number);
                }

                window0.draw(lines);
                window0.display();
            }
            else
                window0.close();

            //------- Визуализация 1 (1.1) (сам граф при T = 1 либо его временная развёртка при T > 1) -------
            vector<CircleShape> draw_vertices = vector<CircleShape>();
            
            for (int i = 0; i < sz; i++)
            {
                CircleShape draw_vertex(25);
                x = rand() % 1350;
                y = rand() % 750;
                draw_vertex.setPosition(x, y);
                draw_vertex.setFillColor(Color::Red);
                draw_vertices.push_back(draw_vertex);
            }

            window1.clear(Color::Black);

            VertexArray lines(LinesStrip, 0);
            int m = 0;
            for (int i = 0; i < sz - 1; i++)
            {
                for (int j = 0; j < sz; j++)
                {
                    if (graph[i][j].c > 0)
                    {
                        lines.append(Vertex(Vector2f(draw_vertices[i].getPosition().x + 12.5, draw_vertices[i].getPosition().y + 12.5)));
                        lines.append(Vertex(Vector2f(draw_vertices[j].getPosition().x + 12.5, draw_vertices[j].getPosition().y + 12.5)));
                        lines[m].color = Color::White;
                        lines[m + 1].color = Color::White;

                        Text draw_c_f;
                        draw_c_f.setFont(font);
                        draw_c_f.setCharacterSize(20);
                        if(graph[i][j].c != DBL_MAX)
                            draw_c_f.setString(deleteUnImportantZeros(to_string(graph[i][j].c)) + " ; " + deleteUnImportantZeros(to_string(graph[i][j].flow)));
                        else
                            draw_c_f.setString("inf ; " + deleteUnImportantZeros(to_string(graph[i][j].flow)));
                        draw_c_f.setFillColor(Color::Yellow);
                        draw_c_f.setPosition((draw_vertices[i].getPosition().x + 12.5 + draw_vertices[j].getPosition().x + 12.5)/2.0, (draw_vertices[i].getPosition().y + 12.5 + draw_vertices[j].getPosition().y + 12.5) / 2.0);
                        draw_c_f.setStyle(sf::Text::Bold);
                        window1.draw(draw_c_f);
                        
                        m += 2;
                    }
                }
            }
            m -= 2;

            for (int i = 0; i < draw_vertices.size(); i++)
            {
                window1.draw(draw_vertices[i]);
                Text draw_vertex_number;
                draw_vertex_number.setFont(font);
                draw_vertex_number.setCharacterSize(25);
                draw_vertex_number.setString(to_string(i));
                draw_vertex_number.setFillColor(Color::Yellow);
                draw_vertex_number.setPosition(draw_vertices[i].getPosition().x + 12.5, draw_vertices[i].getPosition().y + 12.5);
                draw_vertex_number.setStyle(sf::Text::Bold);
                window1.draw(draw_vertex_number);
            }
            Text draw_max_flow;
            draw_max_flow.setFont(font);
            draw_max_flow.setCharacterSize(25);

            //draw_max_flow.setString("V* = " + (maxflow == int(maxflow) ? to_string(int(maxflow)) : to_string(maxflow)));
            draw_max_flow.setString("V* = " + deleteUnImportantZeros(to_string(maxflow)));

            draw_max_flow.setFillColor(Color::Yellow);
            draw_max_flow.setPosition(10, 10);
            draw_max_flow.setStyle(sf::Text::Bold);
            window1.draw(draw_max_flow);
            window1.draw(lines);
            window1.display();

            StopFlow();

            cout << "\nВведите значение, на которое Вы хотели бы уменьшить максимальный поток в сети : ";
            cin >> d;
            cin.clear();
            cin.ignore();

            //cout << "\nВведите множество управляемых вершин (номера вершин от 0 до " << sz - 2 << " )" << endl;
            cout << "\nВведите множество управляемых вершин (номера вершин от 0 до " << N - 2 << " ) : ";
            string S;
            getline(cin, S);
            int i = 0;
            while (S[i] != '\0') {
                if (S[i] != ' ')
                    if(T == 1)
                        managed_vertexes.push_back(S[i] - '0');
                    else {
                        for(auto u: vertextimescanfragmentation[S[i] - '0'])
                            managed_vertexes.push_back(u);
                    }
                i++;
            }
            for (int i = 0; i < sz; i++)
            {
                if (find(managed_vertexes.begin(), managed_vertexes.end(), i) == managed_vertexes.end())
                    not_managed_vertexes.push_back(i);
            }
            cin.clear();

            new_max_flow = PossibleToLowMaxFlow(d, maxflow);

            window1.close();
        }
    }

    k = 0;

    const wchar_t* window2Name = T == 1 ? L"Изменённая сеть" : L"Временная развёртка изменённой сети";
    RenderWindow window2(VideoMode(1400, 800), window2Name, Style::Default);

    window2.setVerticalSyncEnabled(true);

    while (window2.isOpen())
    {
        Event event2;
        while (window2.pollEvent(event2))
        {
            if (event2.type == Event::Closed)
                window2.close();
        }

        if (k == 0)
        {
            k = 1;
            //---------- Визуализация 2 ------------------
            vector<CircleShape> draw_vertices = vector<CircleShape>();

            for (int i = 0; i < sz; i++)
            {
                CircleShape draw_vertex(25);
                x = rand() % 1350;
                y = rand() % 750;
                draw_vertex.setPosition(x, y);
                draw_vertex.setFillColor(Color::Red);
                draw_vertices.push_back(draw_vertex);
            }

            window2.clear(Color::Black);

            VertexArray lines(LinesStrip, 0);
            int m = 0;
            for (int i = 0; i < not_managed_vertexes.size(); i++)
            {
                for (int j = 0; j < not_managed_vertexes.size(); j++)
                {
                    if (not_managed_vertexes[i] != sz - 1 && graph[not_managed_vertexes[i]][not_managed_vertexes[j]].c > 0)
                    {
                        lines.append(Vertex(Vector2f(draw_vertices[not_managed_vertexes[i]].getPosition().x + 12.5, draw_vertices[not_managed_vertexes[i]].getPosition().y + 12.5)));
                        lines.append(Vertex(Vector2f(draw_vertices[not_managed_vertexes[j]].getPosition().x + 12.5, draw_vertices[not_managed_vertexes[j]].getPosition().y + 12.5)));
                        lines[m].color = Color::White;
                        lines[m + 1].color = Color::White;

                        Text draw_c_f;
                        draw_c_f.setFont(font);
                        draw_c_f.setCharacterSize(20);
                        if(graph[not_managed_vertexes[i]][not_managed_vertexes[j]].c != DBL_MAX)
                            draw_c_f.setString(deleteUnImportantZeros(to_string(graph[not_managed_vertexes[i]][not_managed_vertexes[j]].c)) + " ; " + deleteUnImportantZeros(to_string(graph[not_managed_vertexes[i]][not_managed_vertexes[j]].flow)));
                        else
                            draw_c_f.setString("inf ; " + deleteUnImportantZeros(to_string(graph[not_managed_vertexes[i]][not_managed_vertexes[j]].flow)));
                        draw_c_f.setFillColor(Color::Yellow);
                        draw_c_f.setPosition((draw_vertices[not_managed_vertexes[i]].getPosition().x + 12.5 + draw_vertices[not_managed_vertexes[j]].getPosition().x + 12.5) / 2.0, (draw_vertices[not_managed_vertexes[i]].getPosition().y + 12.5 + draw_vertices[not_managed_vertexes[j]].getPosition().y + 12.5) / 2.0);
                        draw_c_f.setStyle(sf::Text::Bold);
                        window2.draw(draw_c_f);

                        m += 2;
                    }
                }
            }
            m -= 2;

            for (int i = 0; i < sz; i++)
                if(find(managed_vertexes.begin(), managed_vertexes.end(), i) == managed_vertexes.end())
                    window2.draw(draw_vertices[i]);
            for (int i = 0; i < not_managed_vertexes.size(); i++)
            {
                Text draw_vertex_number;
                draw_vertex_number.setFont(font);
                draw_vertex_number.setCharacterSize(25);
                draw_vertex_number.setString(to_string(not_managed_vertexes[i]));
                draw_vertex_number.setFillColor(Color::Yellow);
                draw_vertex_number.setPosition(draw_vertices[not_managed_vertexes[i]].getPosition().x + 12.5, draw_vertices[not_managed_vertexes[i]].getPosition().y + 12.5);
                draw_vertex_number.setStyle(sf::Text::Bold);
                window2.draw(draw_vertex_number);
            }
            Text draw_max_flow;
            draw_max_flow.setFont(font);
            draw_max_flow.setCharacterSize(25);
            if(new_max_flow<=maxflow-d)
                draw_max_flow.setString("V* = " + deleteUnImportantZeros(to_string(new_max_flow)) + " <= " + deleteUnImportantZeros(to_string(maxflow)) + " - " + deleteUnImportantZeros(to_string(d)));
            else
                draw_max_flow.setString("V* = " + deleteUnImportantZeros(to_string(new_max_flow)) + " > " + deleteUnImportantZeros(to_string(maxflow)) + " - " + deleteUnImportantZeros(to_string(d)));
            draw_max_flow.setFillColor(Color::Yellow);
            draw_max_flow.setPosition(10, 10);
            draw_max_flow.setStyle(sf::Text::Bold);
            window2.draw(draw_max_flow);
            window2.draw(lines);
            window2.display();
        }
    }

    system("pause");
}
