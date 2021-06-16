
#include "firestore_integration_test.h"

namespace firebase {
namespace firestore {
namespace testing {
namespace {

using DocumentChangeTest = FirestoreIntegrationTest;

TEST_F(DocumentChangeTest, TestDocumentChanges) {
  CollectionReference collection = Collection();
  Query query = collection.OrderBy("a");

  DocumentReference doc1 = collection.Document("1");
  DocumentReference doc2 = collection.Document("2");

  TestEventListener<QuerySnapshot> listener("TestDocumentChanges");
  ListenerRegistration registration = listener.AttachTo(&query);
  Await(listener);
  QuerySnapshot snapshot = listener.last_result();
  EXPECT_EQ(snapshot.size(), 0);

  WriteDocument(doc1, MapFieldValue{{"a", FieldValue::Integer(1)}});
  Await(listener);
  snapshot = listener.last_result();

  std::vector<DocumentChange> changes = snapshot.DocumentChanges();
  EXPECT_EQ(changes.size(), 1);

  EXPECT_EQ(changes[0].type(), DocumentChange::Type::kAdded);
  EXPECT_EQ(changes[0].document().id(), doc1.id());
  EXPECT_EQ(changes[0].old_index(), DocumentChange::npos);
  EXPECT_EQ(changes[0].new_index(), 0);

  WriteDocument(doc2, MapFieldValue{{"a", FieldValue::Integer(2)}});
  Await(listener, 2);
  snapshot = listener.last_result();

  changes = snapshot.DocumentChanges();
  EXPECT_EQ(changes.size(), 1);
  EXPECT_EQ(changes[0].type(), DocumentChange::Type::kAdded);
  EXPECT_EQ(changes[0].document().id(), doc2.id());
  EXPECT_EQ(changes[0].old_index(), DocumentChange::npos);
  EXPECT_EQ(changes[0].new_index(), 1);

  // Make doc2 ordered before doc1.
  WriteDocument(doc2, MapFieldValue{{"a", FieldValue::Integer(0)}});
  Await(listener, 3);
  snapshot = listener.last_result();

  changes = snapshot.DocumentChanges();
  EXPECT_EQ(changes.size(), 1);
  EXPECT_EQ(changes[0].type(), DocumentChange::Type::kModified);
  EXPECT_EQ(changes[0].document().id(), doc2.id());
  EXPECT_EQ(changes[0].old_index(), 1);
  EXPECT_EQ(changes[0].new_index(), 0);
}

}  // namespace
}  // namespace testing
}  // namespace firestore
}  // namespace firebase
