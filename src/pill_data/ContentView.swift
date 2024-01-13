//
//  2_Pillchoose.swift
//  pill_data
//
//  Created by 성수한 on 12/22/23.
//

import SwiftUI
import SwiftData

// Supplement 구조체 수정
struct Supplement: Identifiable, Codable {
    var id = UUID()
    var name: String
    var bestTimeToTake: String
    var methodOfTaking: String
    var ingredients: String
    var benefits: String
}

// SupplementDataStore 클래스 수정
class SupplementDataStore: ObservableObject {
    @Published var supplements: [Supplement] = []

    init() {
        loadSupplementsData()
    }

    // CSV 파일에서 Supplement 데이터 로드 메소드 수정
    private func loadSupplementsData() {
        guard let filePath = Bundle.main.path(forResource: "nutritional_supplements", ofType: "csv") else {
            print("CSV file not found.")
            return
        }
        
        do {
            let data = try String(contentsOfFile: filePath, encoding: .utf8)
            let rows = data.components(separatedBy: "\n")
            
            for row in rows.dropFirst() {
                var columns = [String]()
                var currentField = ""
                var insideQuotes = false

                for character in row {
                    if character == "\"" {
                        insideQuotes.toggle()
                    } else if character == "," && !insideQuotes {
                        columns.append(currentField)
                        currentField = ""
                    } else {
                        currentField.append(character)
                    }
                }
                if !currentField.isEmpty {
                    columns.append(currentField)
                }

                if columns.count == 5 {
                    let supplement = Supplement(
                        name: columns[0],
                        bestTimeToTake: columns[1],
                        methodOfTaking: columns[2],
                        ingredients: columns[3],
                        benefits: columns[4]
                    )
                    supplements.append(supplement)
                }
            }
        } catch {
            print("Error reading CSV file: \(error)")
        }
    }

    // 이름을 포함하는 보충제를 필터링하는 메소드
    func supplements(containing nameSubstring: String) -> [Supplement] {
        supplements.filter { $0.name.localizedCaseInsensitiveContains(nameSubstring) }
    }
}

// 키보드 숨기기 확장
extension View {
    func hideKeyboard() {
        UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder), to: nil, from: nil, for: nil)
    }
}

struct PillIntakeTimeView: View {
    @ObservedObject var dataStore: SupplementDataStore
    var supplementIndex: Int
    @ObservedObject var timeManager: SupplementTimeManager

    var body: some View {
        VStack {
            Text("알약 세부 정보: \(dataStore.supplements[supplementIndex].name)")
            TimeSetView(timeManager: timeManager) // Embed the TimeSetView
            Spacer()
        }
        .padding()
        .navigationTitle("알약 세부 정보")
    }
}




// 기존 ContentView 코드...
struct ContentView: View {
    @StateObject private var store = SupplementDataStore()
    @State private var searchText = ""
    @ObservedObject var timeManager: SupplementTimeManager

    var body: some View {
        ScrollView {
            VStack {
                TextField("영양제를 검색하세요", text: $searchText)
                    .padding()
                    .background(RoundedRectangle(cornerRadius: 15).fill(Color.secondary.opacity(0.1)))
                    .padding()
                    .onSubmit { searchSupplements() }

                Button("검색") { searchSupplements() }
                    .buttonStyle(.borderedProminent)
                    .padding(.bottom)

                ForEach(store.supplements.filter { $0.name.localizedCaseInsensitiveContains(searchText) || searchText.isEmpty }) { supplement in
                    NavigationLink(destination: PillIntakeTimeView(dataStore: store, supplementIndex: store.supplements.firstIndex(where: { $0.id == supplement.id }) ?? 0, timeManager: timeManager)) {
                        SupplementDetailView(supplement: supplement)
                    }
                }
            }
        }
        .navigationTitle("Supplement Search")
    }

    private func searchSupplements() {
        hideKeyboard()
    }
}

// SwiftUI 프리뷰
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView(timeManager: SupplementTimeManager())
    }
}
